#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use FindBin();
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

    DESCRIPTION => <<END,
    $FindBin::Script searches C and C++ files in the specified file arguments
    and reindents the source code using the code indenter "uncrustify".

    By default the result of file F is dumped to file F.indent unless options
    --replace or --no-backup are specified.

    NOTE: Only files with extension (c, h, cpp, hpp, cxx, hxx, C, H) are
    processed.
END
};

#! Expected version of uncrustify
my $UNCRUSTIFY_VERSION = '0.60';

#! Option values
my %OPTIONS;

#! Debug flag
my $DEBUG;

#############################################################################
# Main
#############################################################################

FileUtils::update_files(
    id => 'indent',
    option_values => \%OPTIONS,
    init => \&init,
    file_handler => \&process_file,
    help => $HELP,
);

#############################################################################

#!
# \brief Initialization function
# \details This called just after parsing command line
#
sub init {
    $DEBUG = $OPTIONS{debug};

    # Verify that uncrustify is available and has the right version

    my $version = qx{uncrustify --version};
    chomp($version);

    if( not $version ) {
        die <<END;
Error: $FindBin::Script requires uncrustify version $UNCRUSTIFY_VERSION
END
    }

    if( $version !~ /\s+$UNCRUSTIFY_VERSION$/o ) {
        die <<END;
Error: $FindBin::Script requires uncrustify version $UNCRUSTIFY_VERSION
Found $version which is not suitable.
END
    }

    return;
}

#!
# \brief Run uncrustify on a file
# \param[in] file path to the input file
# \return the text indented by uncrustify
#
sub process_file {
    my($file) = @_;

    my @command = (
        'uncrustify',
        '-q',
        '-c', "$FindBin::Bin/uncrustify.cfg",
        #'-s', '-p', 'uncrustify.log',
        '-l', 'CPP',
    );

    if( defined($file) ) {
        push(@command, '-f', $file);
    }

    # Run uncrustify
    # Get the the result from the uncrustify output

    if( $DEBUG ) {
        print "DEBUG: executing command: @command\n";
    }

    my $text = qx{@command};
    if( $? != 0 ) {
        print "Error: uncrustify failed to process ",
            defined($file) ? $file : "standard input",
            " (return code $?)\n";
        return;
    }

    $text = fix_indent($text);
    return $text;
}

#!
# \brief Fix indentation in a given text
# \param[in] input the input text
# \return the output text
# \details This functions fixes indentation problems left by uncrustify:
#
# 1) Indentation of class headers
#
# Uncrustify has been purposely configured to reformat the constructor
# initializer list by breaking after the first ':' and each ',':
#
# \code
# Class::Class() :
#     init1_(...),
#     init2_(...)
# {
# }
# \endcode
#
#
# Unfortunately this also impacts class headers formatting as follows:
# \code
# class A :
#     public B,
#     public C
# {
# \endcode
#
# We want to reformat them as follows:
#
# \code
# class A : public B, public C {
# \endcode
#
#
# 2) Unexpected indentation problems left by uncrustify:
#
# - closing parens are not indented properly: closing chars are indented
# one level too far and the contents of the parens is not satisfactory
# - extra spaces between a switch case and the case value are not removed
# - empty for(;;;) statements have a space before the closing paren.
#
#
sub fix_indent {
    my($input) = @_;

    # Paren nesting level
    my $paren_level = 0;

    # Pointer to the char after the last newline
    my $last_newline = '';

    # Indentation of the last opening paren
    my $paren_indent = '';

    # Last match
    my $match;

    # Output text
    my $output = '';

    while( $input =~ /(?:
        # Skip preprocessor directive
        \#(?:\\\n|\/\*.*?\*\/|\N)*

        # Skip C++ comment
        |\/\/\N*(?:\n\s*\/\/\N*)*

        # Skip C comment
        |\/\*.*?\*\/

        # Skip string constant
        |"(?:\\.|\N)*?"

        # Skip character constants
        |'(?:\\.|[^\\'])+'

        # Opening paren
        |(?<open_paren>\()

        # Closing paren with leading indent
        |(?<leading_close_paren>\n\h+\))

        # Closing paren
        |(?<close_paren>\h*\))

        # Leading class header
        |(?<class_head>\n\h*(?:class|struct)[^;{]+[;{])

        # Left shift with leading indent
        |(?<left_shift>\n\h+<<)

        # Case labels with extra spaces
        |(?<switch_case>\bcase\h\h+)

        # C++ keywords immediately followed by ::
        |(?:\b(?<keyword>typedef|class|struct|union|enum|public|protected|private|static|inline|extern|virtual|explicit)::)

        # Newline
        |(?<newline>\n)
        )/sx
    ) {
        $match = $&;
        $output .= $`;
        $input = $';

        #print "DEBUG: $match\n";

        if( exists $+{newline} ) {
            # Newline
            $last_newline = $';
            if( $paren_level ) {
                $input =~ s/^\h+/    $paren_indent/;
            }
        }
        elsif( exists $+{open_paren} ) {
            # Opening paren:
            # - Remember the indentation of the first non-white space char in
            # the line
            ++$paren_level;
            if( $paren_level == 1 ) {
                ($paren_indent) = ($last_newline =~ /^(\h*)/);
            } else {
                $paren_indent .= '    ';
            }
        }
        elsif( exists $+{leading_close_paren} ) {
            # Closing paren with leading indent:
            # - Reindent the closing paren at the same level as the line that
            # contains the corresponding opening paren
            # - Decrease indentation
            --$paren_level;
            $match = "\n$paren_indent)";
            $paren_indent =~ s/^    //;
        }
        elsif( exists $+{close_paren} ) {
            # Closing paren
            # - Decrease indentation
            --$paren_level;
            $match = ')';
            $paren_indent =~ s/^    //;
        }
        elsif( exists $+{class_head} ) {
            # Class header with leading indent
            # - Reformat the class header
            $match = reformat_class_header($+{class_head});
        }
        elsif( exists $+{left_shift} ) {
            # Left shift with leading indent
            # - Fix indentation to 4 characters
            # (Uncrustify aligns "<<" vertically by default)
            my($indent) = ($last_newline =~ /^(\h*)/);
            $match = "\n    $indent<<";
        }
        elsif( exists $+{switch_case} ) {
            # Case labels with extra spaces left by uncrustify
            $match = 'case ';
        }
        elsif( exists $+{keyword} ) {
            # C++ keyword immediately followed by ::
            $match = $+{keyword}. ' ::';
        }

        $output .= $match;
    }

    $output .= $input;
    return $output;
}

#!
# \brief Reformat a class header
# \details Removes extra newlines purposely added by the uncrustify
# formatter in class definition headers "class C : public B, ... {"
# \param[in] text text to transform
# \return the transformed text
# \todo The current implementation flattens the whole class header without
# checking the length of the resulting text. We should be a bit smarter than
# that...
#
sub reformat_class_header {
    my($class_head) = @_;

    # Only reformat class definitions (terminated by '{')
    if( $class_head !~ /{$/ ) {
        return $class_head;
    }

    # Preserve leading indentation
    my $indent = '';
    if( $class_head =~ /^\s+/s ) {
        $indent = $&;
        $class_head = $';
    }

    # Compact all spaces
    $class_head =~ s/\s+/ /sg;

    # Fix the missing space before leading :: that has been eaten by uncrustify
    $class_head =~ s/\b(class|struct|union|enum|public|protected|private|virtual)::/$1 ::/gs;

    return $indent . $class_head;
}


