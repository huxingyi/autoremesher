#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use Getopt::Long();
use FindBin();
use File::Find();
use File::Copy();
use Data::Dumper();

package FileUtils;

#############################################################################
# Globals
#############################################################################

#! Result suffix
my $RESULT_SUFFIX = '.mod';

#! Backup suffix
my $BACKUP_SUFFIX = '.mod-backup';

#! List of regular expressions for file exclusion
my @EXCLUDE_REGEXPS;

#! User-supplied file acceptance patterns
my $FILE_PATTERN = qr/\.(?:c|cpp|cxx|C|h|hpp|hxx|H)$/;

#! User-supplied file reordering routine
my $FILE_REORDER;

#! User-supplied handler for processing a file
my $FILE_HANDLER;

#! User-supplied handler for processing the text of a file
my $TEXT_HANDLER;

#! Options values
my %OPTIONS = (
    debug => 0,
    indent => 1,
    backup => 1,
);

#! Debug flags
my $DEBUG;

#!
# \brief Starts the processing
#
sub update_files {
    my(%attrs) = @_;

    # Check user supplied attributes

    if( my $id = $attrs{id} ) {
        $RESULT_SUFFIX = ".$id";
        $BACKUP_SUFFIX = ".$id-backup";
    } else {
        die "Error: must specify an 'id' attribute'\n";
    }

    if( $attrs{file_handler} ) {
        $FILE_HANDLER = $attrs{file_handler};
        if( ref($FILE_HANDLER) ne 'CODE' ) {
            die "Error: attribute 'file_handler' must be a function\n";
        }
    }
    elsif( $attrs{text_handler} ) {
        $TEXT_HANDLER = $attrs{text_handler};
        if( ref($TEXT_HANDLER) ne 'CODE' ) {
            die "Error: attribute 'text_handler' must be a function\n";
        }
    }
    else {
        die "Error: must specify either a 'file_handler' or a 'text_handler' attribute\n";
    }

    if( $attrs{file_reorder} ) {
        $FILE_REORDER = $attrs{file_reorder};
        if( ref($FILE_REORDER) ne 'CODE' ) {
            die "Error: attribute 'file_reorder' must be a function\n";
        }
    }

    if( $attrs{file_pattern} ) {
        $FILE_PATTERN = $attrs{file_pattern};
        if( ref($FILE_PATTERN) ne 'Regexp' ) {
            die "Error: attribute 'file_pattern' must be a Perl Regexp\n";
        }
    }

    my $options;
    if( $options = $attrs{options} ) {
        if( ref($options) ne 'ARRAY' ) {
            die "Error: attribute 'options' must be a Perl ARRAY\n";
        }
    } else {
        $options = [];
    }

    my $option_values;
    if( $option_values = $attrs{option_values} ) {
        if( ref($option_values) ne 'HASH' ) {
            die "Error: attribute 'option_values' must be a Perl HASH\n";
        }
        # Copy local default option values
        while(my($name, $value) = each %OPTIONS) {
            $option_values->{$name} = $value;
        }
    } else {
        $option_values = \%OPTIONS;
    }

    # Decode the command line argument in the user-supplied
    # table of option values

    Getopt::Long::GetOptions(
        $option_values,
        'help',
        'debug:+',
        'verbose',
        'stdout',
        'dry_run|dry-run|n',
        'replace',
        'backup!',
        'exclude=s@',
        @$options
    ) or exit(1);

    # Copy the option values to the local option table
    %OPTIONS = %$option_values;

    # Setup the debug flag
    $DEBUG = $OPTIONS{debug};

    # Check if the help option is specified
    if( $OPTIONS{help}) {
        if( my $help = $attrs{help} ) {
            if( ref($help) ne 'HASH' ) {
                die "Error: attribute 'help' must be a Perl HASH\n";
            }
            help($help);
            exit(0);
        } else {
            die "No help available\n";
        }
    }

    # Call the user supplied init function
    if( my $init = $attrs{init}) {
        if( ref($init) ne 'CODE' ) {
            die "Error: attribute 'init' must be a Perl function\n";
        }
        $init->();
    }

    # Check the validity of the exclude patterns
    check_exclude_patterns();

    # Process input
    if(@ARGV) {
        # Process given input files
        process_files(@ARGV)
    } else {
        # Process standard input
        process_file(undef);
    }
}

#!
# \brief Check the validity of the exclude patterns
# \details Exclude patterns are specified in command line option 'exclude'
#
sub check_exclude_patterns {

    foreach my $pattern (@{$OPTIONS{exclude}}) {

        if( $DEBUG ) {
            print "DEBUG: checking exclude pattern: $pattern\n";
        }

        my $re = eval { qr{$pattern}; };
        if( $@ ) {
            die <<END;
Error: exclude pattern is not a valid regular expression: $pattern
Supported patterns are Perl compatible regular expressions.
END
        }

        push(@EXCLUDE_REGEXPS, $re);
    }

    return;
}

#!
# \brief Collect multiple files or directories
# \details Directories are searched recursively for C and C++ files to
# process. Indivual files are processed if they match the configured file
# patterns (By default C/C++ files extensions)
# \param[in] files list of files or directories to process
# \return list of collected files
#
sub collect_files {
    my(@files) = @_;

    my @collected_files;

    # Filter for the recursive file search

    my $wanted = sub {
        my $file = $_;

        if( not -f $file ) {
            return;
        }

        if( $file !~ $FILE_PATTERN ) {
            return;
        }

        foreach my $re (@EXCLUDE_REGEXPS) {
            $DEBUG > 1 and print "DEBUG: checking $file for pattern: $re\n";
            if( $file =~ $re ) {
                return;
            }
        }

        push(@collected_files, $file);
        return;
    };

    File::Find::find(
        { wanted => $wanted, no_chdir => 1 },
        @files
    );

    return @collected_files;
}

#!
# \brief Process multiple files or directories
# \details Directories are searched recursively for C and C++ files to
# process. Indivual files are processed if they match the configured file
# patterns (By default C/C++ files extensions)
# \param[in] files list of files or directories to process
#
sub process_files {
    my(@files) = @_;

    my @collected_files = collect_files(@files);

    if( $FILE_REORDER ) {
        @collected_files = $FILE_REORDER->(@collected_files);
    }

    foreach my $file (@collected_files) {
        process_file($file);
    }

    return;
}

#!
# \brief Process a single file
# \param[in] file path to the input file
#
sub process_file {
    my($file) = @_;

    if( defined($file) and $OPTIONS{verbose} ) {
        print "Processing: $file\n";
    }

    if( $OPTIONS{dry_run} ) {
        return;
    }

    my $text;
    if( $FILE_HANDLER ) {
        $text = $FILE_HANDLER->($file);
        if( not defined($text) ) {
            return;
        }
    }
    elsif( $TEXT_HANDLER ) {
        $text = defined($file) ? read_file($file) : read_stdin();
        if( not defined($text) ) {
            return;
        }

        my $original_text = $text;

        $text = $TEXT_HANDLER->($file, $text);
        if( not defined($text) ) {
            return;
        }

        if( $text eq $original_text ) {
            if( $OPTIONS{verbose} ) {
                print "File is up to date\n";
            }
            if( not $OPTIONS{stdout} ) {
                return;
            }
        }
    }

    save_result($file, $text);
    return;
}

#!
# \brief Save the result
# \param[in] file path to the output file
# \param[in] text the text to save
#
sub save_result {
    my($file, $text) = @_;

    if( not defined($file) or $OPTIONS{stdout} ) {
        print $text;
        return;
    }

    my $output_file;

    if( not $OPTIONS{replace} ) {
        $output_file = "$file$RESULT_SUFFIX";
    } else {
        $output_file = $file;
        if( $OPTIONS{backup} ) {
            File::Copy::copy($file, "$file$BACKUP_SUFFIX");
        }
    }

    if( $OPTIONS{verbose} ) {
        print "Output: $output_file\n";
    }

    save_file($output_file, $text);
    return;
}


#!
# \brief Reads the contents of a file as an array of lines
# \param[in] file path to the file to read
# \return the contents of the file as an array of lines
#
sub read_file_as_lines {
    my($file) = @_;

    my $fh;
    if( not open($fh, '<', $file) ) {
        print "Error: could not open file $file: $!\n";
        return;
    }

    my @lines = <$fh>;
    close($fh);
    return @lines;
}

#!
# \brief Reads the contents of a file as a single string
# \param[in] file path to the file to read
# \return the contents of the file as a string
#
sub read_file {
    my($file) = @_;

    my $fh;
    if( not open($fh, '<', $file) ) {
        print "Error: could not open file $file: $!\n";
        return;
    }

    local($/);
    my $text = <$fh>;
    close($fh);
    return $text;
}

#!
# \brief Reads standard input as a single string
# \return the contents of the file as a string
#
sub read_stdin {
    local($/);
    my $text = <>;
    return $text;
}

#!
# \brief Reads the contents of a file as a single string
# \param[in] file path to the file to read
# \return the contents of the file as a string
#
sub save_file {
    my($file, @text) = @_;

    my $fh;
    if( not open($fh, '>', $file) ) {
        print "Error: could not open file for writing: $file: $!\n";
        return 0;
    }

    print $fh @text;
    close($fh);
    return 1;
}

#!
# \brief Help
# \param[in] sections map of textual information for the sections NAME
# SYNOPSIS DESCRIPTION OPTIONS and FILES.
#
sub help {
    my($sections) = @_;

    # Fix missing sections

    $sections->{NAME} ||= <<END;
    $FindBin::Script - Undocumented
END

    $sections->{SYNOPSIS} ||= <<END;
    $FindBin::Script [options] [files...]
END

    $sections->{DESCRIPTION} ||= <<END;
    Undocumented
END

    $sections->{OPTIONS} ||= '';
    $sections->{FILES} ||= '';

    print <<END;

NAME
$sections->{NAME}
SYNOPSIS
$sections->{SYNOPSIS}
DESCRIPTION
$sections->{DESCRIPTION}
OPTIONS
    --help
        Print this help and exit.

    --verbose
        Print extra information

    --debug
        Print extra debug output

    --replace
          Replace source files (creates a backup F.indent-backup).

    --no-backup
        In combination with option --replace, --no-backup do not create any
        backup of the input file. This is useful if files are under source
        control.

    --exclude=pattern
        When a directory is searched recursively, excludes the files matching
        the specified pattern (Perl compatible regular expressions). Option
        --exclude can be repeated any number of times to add multiple
        exclusion patterns.

    --dry-run, -n
        Only prints the list of files that would be processed, without
        actually processing them.
$sections->{OPTIONS}
FILES
    List of files and directories to update.
    - File arguments are processed only if they have the requested extension.
    - Directory arguments are searched recursively and all files having the
      requested extension are processed.

    If no input files are specified, the input is read from stdin and
    dumped to standard output

$sections->{FILES}
END
    return;
}

1;


