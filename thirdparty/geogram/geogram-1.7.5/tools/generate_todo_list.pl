#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use FindBin();
use Data::Dumper();
use File::Spec();
use File::Path();
use File::Copy();
use HTML::Entities();
$Data::Dumper::Indent = 1;
$Data::Dumper::Terse = 1;
use lib $FindBin::Bin;
use FileUtils();

#############################################################################
# Globals
#############################################################################

#! Help sections
my $HELP = {
    NAME => <<END,
    $FindBin::Script - Reindent C++ source code
END

    OPTIONS => <<END,

    --csv=file
        Changes the name of CSV data file for the Jenkins Plot Plugin (default
        is todo_list.html). To suppress the generation of the file use
        option -csv=.

    --html=file
        Changes the name of the generated HTML report in the specified file
        (default is todo_list.html). To suppress the generation of the report
        use option -html=.

    --strip-dir=prefix
        Removes the specified prefix from the file path in the HTML report.

    --output-dir=dir
        Generates the reports in the specified directory (default is a
        directory "todo_list" in the curent directory).
END

    DESCRIPTION => <<END,
    $FindBin::Script searches C and C++ files in the specified file arguments
    and generates a HTML report containing all the TODO, TODOC and FIXME
    items. It also generates a CSV data file for the Jenkins Plot Plugin.
    By default reports are generated in a directory "todo_list" in the current
    directory, this can be changed with option --output-dir.

    NOTE: Only files with extension (c, h, cpp, hpp, cxx, hxx, C, H) are
    processed.
END
};

#! Marker list
my @MARKER_LIST = ('TODO', 'FIXME', 'TODOC');

#! Marker labels
my %MARKER_LABEL = (
    'TODOC' => 'Missing documentation (TODOC)',
    'TODO' => 'Things to do (TODO)',
    'FIXME' => 'Things to fix (FIXME)',
);

#! Option values
my %OPTIONS = (
    output_dir => 'todo_list',
    html => 'report.html',
    csv => 'report.csv',
);

#! Map of file anchors
my %FILE_ANCHOR;

#! Map of marker text by file+line
my %MARKER_TEXT;

#! Number of markers per type
my %MARKER_COUNT;

#############################################################################
# Main
#############################################################################

FileUtils::update_files(
    id => 'generate_todo_list',
    option_values => \%OPTIONS,
    options => [
        'output_dir|output-dir=s',
        'html=s',
        'csv=s',
        'strip=s'
    ],
    init => \&init,
    text_handler => \&process_text,
    help => $HELP,
);

# Check specified output directory

generate_csv_data();
generate_html_report();
exit(0);



#############################################################################

#!
# \brief Initialization function
# \details This called just after parsing command line
#
sub init {
    my $dir = $OPTIONS{output_dir};
    if(not $dir) {
        print "Error: no output directory was specified\n";
        exit(1);
    }

    print "DEBUG: dir=$dir\n";
    if(-d $dir) {
        return;
    }

    eval { File::Path::mkpath("$dir/html") };
    if($@) {
        print "Error: output directory could not be created: $dir\n";
        print "$@";
        exit(1);
    }
}

#!
# \brief Searches special patterns in the file
# \param[in] file path to the input file
# \param[in] input text to process
# \return undef
#
sub process_text {
    my($file, $input) = @_;

    my $line = 1;

    while($input =~ /(?:
        # Skip preprocessor directive
        \#(?:\\\n|\/\*.*?\*\/|\N)*

        # Skip C++ comment
        |(?<cpp_comment>\/\/\N*(?:\n\s*\/\/\N*)*)

        # Skip C comment
        |(?<c_comment>\/\*.*?\*\/)

        # Skip string constant
        |"(?:\\.|\N)*?"

        # Skip character constants
        |'(?:\\.|[^\\'])+'
        )/sx
    ) {
        $input = $';
        $line += count_lines($`);

        if(exists $+{cpp_comment}) {
            process_comment($file, $line, "c++", $+{cpp_comment});
        }
        elsif(exists $+{c_comment}) {
            process_comment($file, $line, "c", $+{c_comment});
        }

        $line += count_lines($&);
    }

    # Must return undef to avoid rewriting the file
    return;
}

#!
# \brief Count the number of lines in text
# \param[in] text the input text
# \return the number of \n
#
sub count_lines {
    my($text) = @_;
    return $text =~ tr/\n//;
}

#!
# \brief Process a comment
# \param[in] file the path to the input file
# \param[in] line the starting line of the comment
# \param[in] type the type of the comment (c ot c++)
# \param[in] comment the comment text
#
sub process_comment {
    my($file, $line, $type, $comment) = @_;

    if($comment =~ /\bTODOC\b/) {
        add_marker($file, $line, 'TODOC', $comment);
        return;
    }

    if(not $comment =~ /\b(?:FIXME|TODO)\b/i) {
        return;
    }

    #print "\nDEBUG: Line $line:\n$comment\n";

    if($type eq "c") {
        $comment =~ s{^/\**\s*}{}s;
        $comment =~ s{\s*\*/$}{}s;
        $comment =~ s{^\s*\*}{}mg;
    } else {
        $comment =~ s{^\s*//}{}mg;
    }

    #print "\nDEBUG: raw comment text:\n$comment\n";

    my $marker;
    my $marker_text;
    my $marker_line;
    my @lines = split(/^/, $comment);
    foreach (@lines) {
        #print "DEBUG: comment line: $_\n";
        if(/^\s*$/) {
            if(defined($marker)) {
                add_marker($file, $marker_line, $marker, $marker_text);
                undef $marker;
            }
        }
        elsif(s/^.*?\b(FIXME|TODO)\b:?\s*//i) {
            #print "DEBUG: got marker: $1\n";
            if(defined($marker)) {
                add_marker($file, $marker_line, $marker, $marker_text);
                undef $marker;
            }
            $marker = uc($1);
            $marker_text = $_;
            $marker_line = $line;
        }
        elsif(defined($marker)) {
            $marker_text .= $_;
        }
        ++$line;
    }

    if(defined($marker)) {
        add_marker($file, $marker_line, $marker, $marker_text);
    }

    return;
}

#!
# \brief Adds a marker to the marker table
# \details This updates that FILE table which contains all the file/lines that
# contains markers and the MARKER table with the type and text of the marker.
# \param[in] file the path to the input file
# \param[in] line the line where the marker starts
# \param[in] marker the type of the marker
# \param[in] text the marker text
#
sub add_marker {
    my($file, $line, $marker, $text) = @_;
    $text =~ s/\s+$//;
    $FILE_ANCHOR{$file}{$line} = 1;
    $MARKER_TEXT{$marker}{$file}{$line} = $text;
    $MARKER_COUNT{$marker} += 1;
    return;
}

#!
# \brief Generates a CSV report for the Plot Plugin
#
sub generate_csv_data {
    my($fh, $output_file) = open_report('csv');
    if(not $fh) {
        return;
    }

    print $fh join(",", map { "\"$MARKER_LABEL{$_}\"" } @MARKER_LIST), "\n";
    print $fh join(",", map { $MARKER_COUNT{$_} || 0 } @MARKER_LIST), "\n";
    close($fh);

    print "CSV data saved to $output_file\n";
}

#!
# \brief Generates a HTML report
#
sub generate_html_report {

    # 1) Copy the source files to the output dir

    print "Converting source files to HTML...\n";

    my $strip_prefix = $OPTIONS{strip};
    my $output_dir = $OPTIONS{output_dir};

    foreach my $file (keys %FILE_ANCHOR) {
        my $rel_file = $file;
        if($strip_prefix) {
            $rel_file =~ s{^$strip_prefix}{};
        }

        my $html_file = "$output_dir/html/$rel_file.html";
        my $html_file_dir = File::Basename::dirname($html_file);

        if(not -d $html_file_dir) {
            eval { File::Path::mkpath($html_file_dir) };
            if($@) {
                print "Error: failed to create directory $html_file_dir $@\n";
                next;
            }
        }

        if(not convert_source_file_to_html($rel_file, $file, $html_file)) {
            print "Error: failed to generate HTML file: $html_file\n";
            next;
        }
    }

    # 2) Generate the report

    print "Generating HTML report...\n";

    my($fh, $output_file) = open_report('html');
    if(not $fh) {
        return;
    }

    print $fh <<'END';
<html>
    <head>
        <title>Marker Summary</title>
        <style type="text/css">
body {
    background-color: white;
    font-family: verdana,helvetica,arial,sans-serif;
}
table {
    border: 1px solid black;
    border-collapse: collapse;
}
tr.header {
    background-color: #ddd;
}

        </style
    </head>
    <body>
        <h3>Source code markers summary</h3>
        <table class="table" border="1" cellspacing="0" cellpadding="4">
            <tbody>
END

    foreach my $marker (@MARKER_LIST) {
        my $count = $MARKER_COUNT{$marker} || 0;
        print $fh <<END;
                <tr>
                    <td><a href="#$marker">$MARKER_LABEL{$marker}</a></td>
                    <td>$count</td>
                </tr>
END
    }

    print $fh <<'END';
            </tbody>
        </table>
END

    foreach my $marker (@MARKER_LIST) {
        print $fh <<END;
        <a name="$marker"><h3>$MARKER_LABEL{$marker}</h3></a>
        <table class="table" border="1" cellspacing="0" cellpadding="4">
            <tbody>
                <tr class="header">
                    <td>File</td>
                    <td>What</td>
                </tr>
END

        my $marker_text_by_file = $MARKER_TEXT{$marker};
        foreach my $file (sort keys %$marker_text_by_file) {

            my $rel_file = $file;
            if($strip_prefix) {
                $rel_file =~ s{^$strip_prefix}{};
            }

            my $marker_text_by_line = $marker_text_by_file->{$file};
            foreach my $line (sort { $a <=> $b } keys %$marker_text_by_line) {
                my $marker_text = HTML::Entities::encode($marker_text_by_line->{$line});
                print $fh <<END;
                <tr>
                    <td nowrap><a href="html/$rel_file.html#$line">$rel_file:$line</a></td>
                    <td>$marker_text</td>
                </tr>
END
            }
        }

        print $fh <<'END';
            </tbody>
        </table>
END
    }

    print $fh <<'END';
    </body>
</html>
END

    print "HTML report saved to $output_file\n";
}

#!
# \brief Converts a source file to HTML
# \param[in] file path to the input source file
# \param[in] html_file path to the output HTML file
# \retval true if the file was succesfully converted
# \retval false otherwise
#
sub convert_source_file_to_html {
    my($title, $file, $html_file) = @_;

    my @input = FileUtils::read_file_as_lines($file);
    if(not @input) {
        return;
    }

    my @output =(<<END);
<html>
    <head>
        <title>$title</title>
        <style type="text/css">
body {
    font-family: courier-new,courier,fixed;
    font-size 9;
    white-space: pre;
}
a {
    background-color: orange;

}
        </style>
    </head>
    <body>
END

    my $lineno = 1;
    foreach my $line (@input) {
        $line = sprintf("%6d: %s", $lineno, HTML::Entities::encode($line));
        if($FILE_ANCHOR{$file}{$lineno}) {
            push(@output, "<a class=\"anchor\" name=\"$lineno\">$line</a>");
        } else {
            push(@output, $line);
        }
        $lineno += 1;
    }

    push(@output, <<END);
    </body>
</html>
END

    return FileUtils::save_file($html_file, @output);
}

#!
# \brief Gets a report file handle
# \details Determines the filename for the report \p report, optionally in the
# output directory specified on the command line, and opens the file for
# writing.
# \param[in] report the type of the report
# \return a couple (file handle, output path)
#
sub open_report {
    my($report) = @_;

    my $path = $OPTIONS{$report};
    if(not $path) {
        return;
    }

    if(my $dir = $OPTIONS{output_dir}) {
        $path = "$dir/$path";
    }

    my $fh;
    if(not open($fh, '>', $path)) {
        print STDERR "Failed to write file: $path: $!\n";
        return;
    }
    
    return ($fh, $path);
}
