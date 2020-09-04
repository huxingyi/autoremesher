#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use File::Basename();
use FindBin();
use lib $FindBin::Bin;
use FileUtils();

#############################################################################
# Globals
#############################################################################

#! Help sections
my $HELP = {
    NAME => <<END,
    $FindBin::Script - Fix documentation in C++ source code
END

    DESCRIPTION => <<END,
    $FindBin::Script searches C and C++ files in the specified file arguments
    and does the following:
    - it adds missing doxygen comments for undocumented C++ constructs.
    - it fixes existing doxygen comment (only with option --fix-comments)

    By default the result of file F is dumped to file F.fixdoc unless options
    --replace or --no-backup are specified.

    NOTE: Only files with extension (c, h, cpp, hpp, cxx, hxx, C, H) are
    processed.
END

    OPTIONS => <<END,

    --fix-file-comments
        Fixes missing \\file doxygen comments in the header files. It uses a
        heuristic to determine the right place to insert the missing comment:
        - before the top-level namespace if any
        - otherwise after the last #include

    --fix-comments
        Fixes existing doxygen comments in the source code. It does the
        following changes:
        
        - Changes doxygen directives using a leading '\' (eg: \@brief -> \\brief)
        
        - Capitalizes the first letter of the word after after sections
          \\brief, \\details, \\note...
        
        - Adds missing \\param roles (eg: \\param name -> \\param[in] name)

        - Removes extra leading spaces in \\code...\\endcode sections (while
          keeping code indentation).

        - Aggressively removes all extra leading spaces (only with option --fix-spaces)

    --fix-spaces
        Only with option --fix-comments. Aggressively removes all extraneous
        leading spaces in the doxygen comments. For instance, the following
        comment:

        /*
         * \\param[in] param_name a very long description
         *      that wraps on the next line
         */

        will be changed in:

        /*
         * \\param[in] param_name a very long description
         *  that wraps on the next line
         */

        Warning: This option can have some unwanted effects when the
        documentation has been carefully indented (such as in lists and nested
        lists):

        /*
         * List example:
         * - item 1
         *   item 1 continued...
         *   - sub item 1
         *     sub item 1 continued...
         *   - sub item 2
         */

        will be changed in:

        /*
         * List example:
         * - item 1
         *  item 1 continued...
         *  - sub item 1
         *  subitem 1 continued...
         *  - sub item 2
         */

        which is clearly not wanted.

        Warning: it is highly recommended to review the changes after
        comments have been changed using this option.
END
};

#! Global regular expressions
my $RE_ANGLES;
my $RE_PARENS;
my $RE_BRACKETS;

#! Map of C++ declarations (used with option --fix-doc only)
my %CXX_DECLS;

#! Map of the .h files added from the .cpp files
my %ADDED_HFILES;

#! Option values
my %OPTIONS;

#! Debug flag
my $DEBUG;

#! Extra debug flag
my $EXTRA_DEBUG;

#############################################################################
# Main
#############################################################################

FileUtils::update_files(
    id => 'fixdoc',
    option_values => \%OPTIONS,
    options => [
        'fix_file_comments|fix-file-comments',
        'fix_comments|fix-comments',
        'fix_spaces|fix-spaces',
    ],
    init => \&init,
    file_reorder => \&reorder_files,
    text_handler => \&process_text,
    help => $HELP,
);

#############################################################################

#!
# \brief Initialization function
# \details This called just after parsing command line
#
sub init {
    $DEBUG = $OPTIONS{debug};
    $EXTRA_DEBUG = $DEBUG > 1;

    $RE_ANGLES = balanced_expression_regexp('<', '>');
    $RE_PARENS = balanced_expression_regexp('\(', '\)');
    $RE_BRACKETS = balanced_expression_regexp('\[', '\]');
    return;
}

#!
# \brief Reorders files collected by FileUtils
# \param[in] files files collected by FileUtils
# \details
# Rebuilds the list of initial collected files ordered as follows:
# - first header files (potentially augmented with some header files)
# - then body files
# \return the list of reordered files
#
sub reorder_files {
    my(@files) = @_;

    # 1) Extract the list of header files

    my @hfiles = grep(/\.(?:h|hpp|hxx|H)$/, @files);
    my %hfile_map = map { $_ => 1 } @hfiles;

    # 2) For each body file, make sure that the corresponding header file is in
    # the header file list, to avoid adding duplicate documentation to the
    # function implementation.

    my @cfiles = sort grep(!/\.(?:h|hpp|hxx|H)$/, @files);
    foreach my $cfile (@cfiles) {
        foreach my $ext ('.h', '.hpp', '.hxx', '.H') {
            (my $hfile = $cfile) =~  s/\.(?:c|cpp|cxx|C)$/$ext/;
            if( -f $hfile ) {
                if( not exists $hfile_map{$hfile} ) {
                    $hfile_map{$hfile} = 1;
                    $ADDED_HFILES{$hfile} = 1;
                }
                last;
            }
        }
    }

    # Rebuild the list of initial collected files ordered as follows:
    # - first header files (potentially augmented with some header files)
    # - then body files

    @hfiles = sort keys %hfile_map;
    return (@hfiles, @cfiles);
}

#!
# \brief Fix documentation in a file
# \param[in] file path to the input file
# \param[in] text the file contents
# \return the updated text.
#
sub process_text {
    my($file, $text) = @_;

    $text = fix_missing_doc($text);

    if( $ADDED_HFILES{$file} ) {
        # File was added by reorder_files() to make the pair with a .cpp file
        # Tell FileUtils not to save this file
        return undef;
    }

    if($OPTIONS{fix_file_comments}) {
        if( not defined($file) ) {
            $text = fix_file_comments("standard-input", $text);
        } elsif( $file =~ /\.(?:h|hpp|hxx|H)$/ ) {
            $text = fix_file_comments($file, $text);
        }
    }

    if( $OPTIONS{fix_comments} ) {
        $text = fix_comments($file, $text);
    }

    return $text;
}

#!
# \brief Fix missing file comments
# \details This uses a heuristics to determine the right place of the file
# comment:
# - before the top-level namespace if any
# - otherwise after the last #include
# \param[in] input the input text
# \return the output text
#
sub fix_file_comments {
    my($file, $input) = @_;

    # Remove the path prefix up to src/lib/
    $file =~ s{^.*\bsrc/lib/}{};

    # File comment to insert
    my $comment = <<END;

/**
 * \\file $file
 * \\brief TODOC
 */
END

    # Output text
    my $output = '';

    # Last match
    my $match;

    # Insertion position of the comment (if needed)
    my $insert_pos;

    # Local regular expressions
    # Matches a doxygen comment occuring at the end of a text
    # Note: trivial regexp \/\*.*?\*\/ does not work when attempting to
    # match the end of a string (it matches too many characters, even comment
    # boundaries!). So we must use a more clever regexp that excludes the
    # possibility to match across comment boundaries:
    my $_trailing_doxygen_comment = qr/\n\/\*[\*!]\s+([^*]|\*+[^*\/])*\*+\/\s*$/s;


    while( $input =~ /
        (?:
            # Include directives
            (?<include>\#\h*include[^\n]+)

            # Doxygen comment
            |(?<doxygen>\/\*[*!]\s+.*?\*\/)

            # Toplevel namespace declaration
            |(?<namespace>\nnamespace\h+\w+\h*{)
        )
        /sx
    ) {
        $match = $&;
        $output .= $`;
        $input = $';

        $EXTRA_DEBUG and print "DEBUG: match=$match\n";

        if( exists $+{include} ) {
            $output .= $match;
            $insert_pos = length($output)+1;
            next;
        }

        if( exists $+{doxygen} ) {
            $output .= $match;
            if( $match =~ /\\file/ ) {
                # File already has a file comment
                undef $insert_pos;
                last;
            }
            next;
        }

        if( exists $+{namespace} ) {
            if( $output =~ $_trailing_doxygen_comment ) {
                $insert_pos = length($`);
            } else {
                $insert_pos = length($output);
            }
            $output .= $match;
            last;
        }
    }

    $output .= $input;

    if( $insert_pos ) {
        $output = substr($output, 0, $insert_pos)
            . $comment
            . substr($output, $insert_pos);
    }

    return $output;
}


#!
# \brief Fix missing class and function documentation
# \param[in] input the input text
# \return the output text
#
sub fix_missing_doc {
    my($input) = @_;

    # Stack of nested scopes
    my @scopes;

    # Stack of nested scope flags
    my @scope_doc = (1);

    # Pointer to the char after the last newline
    #my $last_newline = '';

    # Indentation of the last opening paren
    my $paren_indent = '';

    # Last match
    my $match;

    # Output text
    my $output = '';

    # Local regular expressions

    # Matches an optional nested-name-specifier (left part of a qualified id)
    my $_scope_opt = qr/(?:\b(\w+)(?:\s*$RE_ANGLES)?\s*::\s*)*/s;

    # Matches a doxygen comment occuring at the end of a text
    # Note: trivial regexp \/\*.*?\*\/ does not work when attempting to
    # match the end of a string (it matches too many characters, even comment
    # boundaries!). So we must use a more clever regexp that excludes the
    # possibility to match across comment boundaries:
    my $_trailing_doxygen_comment = qr/\/\*[\*!]\s+([^*]|\*+[^*\/])*\*+\/\s*$/s;

    while( $input =~ /
        (?(DEFINE)
            (?<_cxx_comment>\/\/\N*(?:\n\s*\/\/\N*)*)
            (?<_c_comment>\/\*.*?\*\/)
            (?<_string_constant>"(?:\\.|\N)*?")
            (?<_char_constant>'(?:\\.|[^\\'])+')
            (?<_cpp_directive>\#(?:\\\n|(?&_c_comment)|\N)*)
            (?<_constants>(?:
                (?&_cpp_directive)|
                (?&_cxx_comment)|
                (?&_c_comment)|
                (?&_string_constant)|
                (?&_char_constant))
            )
            (?<_parens>(\((?:(?&_constants)|[^()]|(?-1))*\)))
            (?<_braces>({(?:(?&_constants)|[^{}]|(?-1))*}))
            (?<_angles>(<(?:(?&_constants)|[^<>]|(?-1))*>))
            (?<_scope_opt>(?:\b(\w+)(?:\s*(?&_angles))?\s*::\s*)*)
            (?<_abs_scope_opt>(?:::\s*)?(?&_scope_opt))
            (?<_qualified_name>(?&_abs_scope_opt)\w+(?:\s*(?&_angles))?)
            (?<_initializer>(?&_qualified_name)\s*(?&_parens))
            (?<_struct_key>\b(?:class|struct|union|enum)\b)
            #(?<_struct_head>(?&_struct_key)[^;{]+[;{])
            (?<_base_class>
                (?:\s*\b(?:public|protected|private|virtual)\b)*
                \s*(?&_qualified_name)
            )
            (?<_base_clause_opt>(?:\s*:\s*(?&_base_class)(?:\s*,\s*(?&_base_class))*)?)
        )
        (?:
            # Constants
            (?&_constants)

            # Namespace
            |(?<namespace>\n+\h*namespace\b(?:\s+(?<namespace_name>\w+\b))?[^;{]*{)

            # Friend, using
            |\n+\h*(?:friend|using)\b[^;]+;

            # Declaration
            |(?<decl>
                \n+\h*
                (?:template\s*(?&_angles)\s*)*
                (?:

                    # Struct, class, union, enum
                    (?<struct>
                        (?<struct_key>(?&_struct_key))
                        (?:
                            (?:\s*(?<struct_name>(?&_qualified_name)))+
                            (?&_base_clause_opt)
                        )?
                        \s*[;{]
                    )

                    # Typedef
                    |(?<typedef>
                        typedef\s+
                        (?:
                            (?<struct_key>(?&_struct_key))
                            (?:
                                (?:\s*(?<struct_name>(?&_qualified_name)))+
                                (?&_base_clause_opt)
                            )?
                            \s*[;{]

                            |[^;]+;
                        )
                    )

                    # Function
                    |(?<function>
                        (?<fdecl>
                            (?:
                                (?:
                                    \*
                                    |\&
                                    |::
                                    |\boperator\b\s*(?:\(\)|\S+)
                                    |\b\w+\b
                                    |\~\w+\b
                                    |(?&_angles)
                                )
                                \s*
                            )+
                        )
                        (?<fargs>(?&_parens))
                        (?:\s*:\s*(?&_initializer)(?:\s*,\s*(?&_initializer))*)?
                    )
                )
            )

            # Braced expression
            |(?&_braces)

            # Paren expression
            |(?&_parens)

            # Closing brace
            |(?<close_brace>})

            # Newline
            #|(?<newline>\n)
        )
        /sx
    ) {
        $match = $&;
        $output .= $`;
        $input = $';

        $EXTRA_DEBUG and print "DEBUG: scope=[@scopes]\n";

        if( exists $+{decl} ) {
            if( exists $+{struct} ) {
                # Struct, class, ... definition
                $EXTRA_DEBUG and print "DEBUG: struct: $match\n";

                # Save previous match variables before running a new regexp
                my $struct_name = $+{struct_name};
                my $struct_key = $+{struct_key};

                # Check if the declaration defines a new struct
                if( $match =~ /{$/ ) {

                    # Check if we have a preceding doxygen comment
                    # (only if we are in a "doc-enabled" scope)
                    if( $scope_doc[-1] and $output !~ $_trailing_doxygen_comment ) {
                        $EXTRA_DEBUG and print "DEBUG: adding doxygen documentation\n";
                        $match = add_struct_documentation($match, $struct_key, $struct_name);
                    }

                    # Update scope stack
                    my $scope_name = $struct_name || $struct_key;
                    push(@scopes, $scope_name);

                    # Do not document items in class template specializations
                    my $can_document = $scope_name !~ />$/;
                    push(@scope_doc, $scope_doc[-1] && $can_document);
                }
            }
            elsif( exists $+{typedef} ) {
                # Typedef
                $EXTRA_DEBUG and print "DEBUG: typedef: $match\n";

                # Save previous match variables before running a new regexp
                my $struct_name = $+{struct_name};
                my $struct_key = $+{struct_key};

                # Check if we have a preceding doxygen comment
                # (only if we are in a "doc-enabled" scope)
                if( $scope_doc[-1] and $output !~ $_trailing_doxygen_comment ) {
                    $EXTRA_DEBUG and print "DEBUG: adding doxygen documentation\n";
                    $match = add_typedef_documentation($match);
                }

                # Check if the declaration defines a new struct
                if( $match =~ /{$/ ) {
                    # Update scope stack
                    my $scope_name = $struct_name || $struct_key;
                    push(@scopes, $scope_name);

                    # Do not document items in class template specializations
                    my $can_document = $scope_name !~ />$/;
                    push(@scope_doc, $scope_doc[-1] && $can_document);
                }
            }
            elsif( exists $+{function} ) {
                # Function
                $EXTRA_DEBUG and print "DEBUG: function: $match\n";
                $EXTRA_DEBUG and print "DEBUG: fdecl = $+{fdecl}\n";
                $EXTRA_DEBUG and print "DEBUG: fargs = $+{fargs}\n";

                # Save previous match variables before running a new regexp
                my $fdecl = $+{fdecl};
                my $fargs = $+{fargs};
                my $ftype;
                my $fname;

                # Extract the function name from the function decl.
                if( $fdecl =~ /
                    \s*(
                        $_scope_opt
                        (?:
                            \boperator\b\s*(?:\(\)|\S+)
                            |\~?\b\w+
                        )
                    )
                    \s*$
                    /sx
                ) {
                    $ftype = $`;
                    $fname = $1;
                    $EXTRA_DEBUG and print "DEBUG: ftype = $ftype\n";
                    $EXTRA_DEBUG and print "DEBUG: fname = $fname\n";
                }

                if( $fname ) {
                    if( register_function(\@scopes, $ftype, $fname, $fargs) ) {
                        # Check if we have a preceding doxygen comment
                        # (only if we are in a "doc-enabled" scope)
                        if( $scope_doc[-1] and $output !~ $_trailing_doxygen_comment ) {
                            $EXTRA_DEBUG and print "DEBUG: adding doxygen documentation\n";
                            $match = add_function_documentation($output, $ftype, $fname, $fargs, $match);
                        }
                    } else {
                        # The function has been already registered:
                        # A doxygen documentation template has been already
                        # added to the first occurrence of the function. Thus
                        # we can remove any duplicate documentation template
                        # that precedes this occurrence of the function.
                        if( $output =~ s/$_trailing_doxygen_comment//s ) {
                            my $comment = $&;
                            if( $comment =~ /\bTODOC\b/s ) {
                                # The comment is a documentation template:
                                # we can safely remove it
                                $EXTRA_DEBUG and print "DEBUG: removing doxygen documentation\n";
                                $output =~ s/\s+$/\n\n/s;
                                $match =~ s/^\n+//s;
                            }
                        }
                    }
                }
            }
        }
        elsif( exists $+{namespace} ) {
            # Namespace
            $EXTRA_DEBUG and print "DEBUG: namespace: $match\n";
            push(@scopes, $+{namespace_name} || 'namespace');
            push(@scope_doc, 1);
        }
        elsif( exists $+{close_brace} ) {
            # Closing brace
            $EXTRA_DEBUG and print "DEBUG: close_brace: $match\n";
            pop(@scopes);
            pop(@scope_doc);
        }
        else {
            $EXTRA_DEBUG and print "DEBUG: match: $match\n";
        }

        $output .= $match;
    }

    $output .= $input;
    return $output;
}

#!
# \brief Register a fully qualified name
# \param[in] scopes list of active C++ scopes
# \param[in] ftype return type of the function
# \param[in] fname name of the function
# \param[in] fargs function arguments
# \retval true if the name was successfully registered
# \retval false if the name was already registered
#
sub register_function {
    my($scopes, $ftype, $fname, $fargs) = @_;

    if( $fname eq 'main' ) {
        # Do not register main programs
        return 0;
    }

    $ftype = canonical_function_type($ftype);
    if( $ftype ne "" ) {
        $ftype .= " ";
    }

    my $scope = join("::", @$scopes);

    my $qualified_name =
        $ftype
        . join("::", @$scopes, $fname)
        . canonical_function_arguments($fargs);

    if( exists $CXX_DECLS{$qualified_name} ) {
        $EXTRA_DEBUG and print "DEBUG: qname already registered: $qualified_name\n";
        return 0;
    }

    $EXTRA_DEBUG and print "DEBUG: register name: $qualified_name\n";
    $CXX_DECLS{$qualified_name} = 1;
    return 1;
}

#!
# \brief Canonicalize a function argument list
# \details This function computes the canocical form of a function argument
# list, that is: the signature of the function without the argument names and
# without unnecessary spaces.
# \param[in] fargs list of arguments enclosed in parens
# \return the canonical form of the arguments
#
sub canonical_function_arguments {
    my($fargs) = @_;

    # Replace leading and trailing parens from the arg list by commas.
    # This is a trick to make all arguments appear between a pair of
    # commas. This simplifies all the regular expressions.

    $fargs =~ s/^\s*\(\s*/,/s;
    $fargs =~ s/\s*\)\s*$/,/s;
    
    if( $fargs eq ",," ) {
        return "()";
    }

    #
    # ALGORITHM
    #
    # The main goal in computing the canonical form is to remove argument
    # names, that is identifiers preceding charset "[,)"
    #
    # This is quite simple with named argument declarations:
    #
    # ..., const Type x, ... -> ..., const Type, ...
    # ..., Type const x, ... -> ..., Type const, ...
    # ..., Type* x, ...      -> ..., Type*, ...
    # ..., Type x[], ...     -> ..., Type[], ...
    #
    # Unnamed declarations are more difficult to handle. We must distinguish 2
    # cases a) and b):
    #
    # a) Unnamed declarations *not* terminated by an identifier. These
    # declarations are trivial and do not need any special processing:
    #
    # ..., Type*, ...        -> ..., Type*, ...
    # ..., Type const &, ... -> ..., Type const&, ...
    # ..., Type[], ...       -> ..., Type[], ...
    #
    # b) Unnamed declarations terminated by an identifier. 
    #
    # ..., Type, ...         -> ..., Type, ...
    # ..., const Type, ...   -> ambiguous: is Type a typename or an argname?
    # ..., Type const, ...   -> ambiguous: is const a typename or an argname?
    #
    # The difficulty here is to make the difference between a parameter name
    # and a type name when the typename is associated with cv-qualifiers
    # (const, volatile).
    #
    # The ideal would be to remove all cv-qualifiers to make argument
    # declarations non ambiguous. Instead we will use a marking algorithm to
    # preserve the type-expression.
    #
    # 1) Mark all cv-qualifiers that do not start a type expression
    #
    # a) ",Type,"              -> ",Type,"
    # b) ",const Type,"        -> ",const Type,"
    # c) ",Type const,"        -> ",Type const##,"      <-- HERE
    # d) ",Type arg,"          -> ",Type arg,"
    # e) ",const Type arg,"    -> ",const Type arg,"
    # f) ",Type const arg,"    -> ",Type const## arg,"  <-- HERE
    #
    # In case c) this guarantees that "const##" will not be removed in step 4
    #
    # 2) Mark all other cv-qualifiers with a trailing comma:
    #
    # a) ",Type,"              -> ",Type,"
    # b) ",const Type,"        -> ",const@@, Type,"     <-- HERE
    # c) ",Type const##,"      -> ",Type const##,"
    # d) ",Type arg,"          -> ",Type arg,"
    # e) ",const Type arg,"    -> ",const@@, Type arg," <-- HERE
    # f) ",Type const## arg, " -> ",Type const## arg,"
    #
    # In case b), adding a comma after const virtually "removes" the
    # cv-qualifier from the argument declaration. So the rest of the
    # declaration is reduded to the non qualified "Type"
    #
    # 3) Mark all identifiers that follow charset "(,:" and that precede
    # charset ",)". They are considered as isolated typenames.
    #
    # a) ",Type,"              -> ",Type##,"            <-- HERE
    # b) ",const@@, Type,"     -> ",const@@, Type##,"   <-- HERE
    # c) ",Type const##,"      -> ",Type const##,"
    # d) ",Type arg,"          -> ",Type arg,"
    # e) ",const@@, Type arg," -> ",const@@, Type arg,"
    # f) ",Type const## arg,"  -> ",Type const## arg,"
    #
    # In case a) and b) this guarantees that "Type##" will not be removed in
    # step 4
    #
    # Note: the char ":" in charset "(,:" guarantees that the last
    # part of a fully qualified typename will not be removed in step 4:
    #
    # g) ",Nested::Type,"      -> ",Nested::Type##,"
    #
    # 4) At this point, typenames and/or cv-qualifiers that end an argument
    # declaration have been marked. We can remove all identifiers that precede
    # charset ",)["
    #
    # a) ",Type##,"            -> ",Type##,"
    # b) ",const@@, Type##,"   -> ",const@@, Type##,"
    # c) ",Type const##,"      -> ",Type const##,"
    # d) ",Type arg,"          -> ",Type,"           <-- HERE
    # e) ",const@@, Type arg," -> ",const@@, Type,"  <-- HERE
    # f) ",Type const## arg,"  -> ",Type const##,"   <-- HERE
    #
    # 5) Remove all markers.
    #
    # a) ",Type##,"            -> ",Type,"
    # b) ",const@@, Type##,"   -> ",const Type,"
    # c) ",Type const##,"      -> ",Type const,"
    # d) ",Type arg,"          -> ",Type,"
    # e) ",const@@, Type arg," -> ",const Type,"
    # f) ",Type const## arg,"  -> ",Type const,"
    #

    # Remove argument default values
    # Note: This simple-minded regexp assumes that default values do not
    # contains commas ","

    $fargs =~ s/\s*=.+?,/,/sg;

    # 1) Mark all cv-qualifiers that do not start a type expression
    
    $fargs =~ s/[^\(,\s]\s*(?:const|volatile)\b/$&##/sg;

    $EXTRA_DEBUG and print "DEBUG: fargs.1 = [[$fargs]]\n";

    # 2) Mark all other cv-qualifiers with a trailing comma:

    $fargs =~ s/\b(const|volatile)\b([^#])/$1@@,$2/sg;

    $EXTRA_DEBUG and print "DEBUG: fargs.2 = [[$fargs]]\n";

    # 3) Protect isolated words enclosed in punctuation

    while( $fargs =~ s/([,:\(]\s*\w+)(\s*[,\)])/$1##$2/sg ) { }

    $EXTRA_DEBUG and print "DEBUG: fargs.3 = [[$fargs]]\n";

    # 4) Remove all identifiers that precede charset ",)["

    $fargs =~ s/\s*\w+\s*([,\)\[])/$1/sg;

    $EXTRA_DEBUG and print "DEBUG: fargs.4 = [[$fargs]]\n";

    # 5) Remove protection markers

    $fargs =~ s/@@,//sg;
    $fargs =~ s/##//sg;

    $EXTRA_DEBUG and print "DEBUG: fargs.5 = [[$fargs]]\n";

    # Remove added leading and trailing commas

    $fargs =~ s/^\s*,\s*//s;
    $fargs =~ s/\s*,\s*$//s;

    # Remove all spaces around punctuation chars
    
    $fargs =~ s/\s*([,*&\(\)\[\]<>])\s*/$1/sg;

    return "($fargs)";
}

#!
# \brief Canonicalize a function return type
# \param[in] ftype string that contains the function typedef
# \return the canonical form of the function type
#
sub canonical_function_type {
    my($ftype) = @_;

    $ftype =~ s/^\s+//g;
    $ftype =~ s/\s+$//g;
    $ftype =~ s/\b(?:static|extern|inline|virtual|explicit|VORPALIB_API)\b\s*//sg;
    $ftype =~ s/\s*([,*&\(\)\[\]<>])\s*/$1/sg;
    $ftype =~ s/^\s+//s;
    $ftype =~ s/\s+$//s;
    $ftype =~ s/\s+/ /sg;
    return $ftype
}


#!
# \brief Add a leading doxygen comment to a typedef declaration
# \param[in] text string starting with a struct or typedef definition
# \return the documented declaration
#
sub add_typedef_documentation {
    my($text) = @_;

    # Extract the leading newlines and indentation
    my($newlines, $indent) = ($text =~ /^(\n+)(\h*)/s);
    $text = $';

    # Add an extra newline if needed
    if( length($newlines) == 1 ) {
        $newlines .= "\n";
    }

    # Return the documented declaration
    return "$newlines$indent/** \\brief TODOC */
$indent$text";
}

#!
# \brief Add a leading doxygen comment to a struct declaration
# \param[in] text string starting with a struct or typedef definition
# \param[in] struct_key type of the stucture (struct, class, enum, union)
# \param[in] struct_name name of the stucture
# \return the documented declaration
#
sub add_struct_documentation {
    my($text, $struct_key, $struct_name) = @_;

    # Extract the leading newlines and indentation
    my($newlines, $indent) = ($text =~ /^(\n+)(\h*)/s);
    $text = $';

    # Add an extra newline if needed
    if( length($newlines) == 1 ) {
        $newlines .= "\n";
    }

    if( defined($struct_name) and $struct_name =~ />$/ ) {
        # The struct is a template specialization
        # Return a special documentation
        $struct_name =~ s/<.+$//;
        return "$newlines$indent/**
$indent * \\brief Specialization of $struct_key $struct_name
$indent * \\see $struct_name
$indent */
$indent$text";
    }

    # Return the documented declaration
    return "$newlines$indent/**
$indent * \\brief TODOC
$indent * \\details TODOC
$indent */
$indent$text";
}

#!
# \brief Add a leading doxygen comment to a function declaration
# \param[in] ftype full text of the function type (including storage and
#  declaration qualifiers).
# \param[in] fname name of the function
# \param[in] fargs list of arguments enclosed in parens
# \param[in] text string starting with a struct or typedef definition
# \return the documented function declaration
#
sub add_function_documentation {
    my($output, $ftype, $fname, $fargs, $text) = @_;

    # Extract the leading newlines and indentation
    
    my($newlines, $indent) = ($text =~ /^(\n+)(\h*)/s);
    $text = $';

    # Add an extra newline before the comment if needed
    # (Note: except when the function follows access specifiers)

    if( length($newlines) == 1 and $output !~ /\b(?:public|protected|private)\s*:\s*$/s ) {
        $newlines .= "\n";
    }

    # Replace leading and trailing parens from the arg list by commas.
    # This is a trick to make all arguments appear between a pair of
    # commas. This simplifies all the regular expressions.

    $fargs =~ s/^\s*\(\s*/,/s;
    $fargs =~ s/\s*\)\s*$/,/s;

    # Remove argument default values
    # Note: This simple-minded regexp assumes that default values do not
    # contains commas ","

    $fargs =~ s/\s*=.+?,/,/sg;

    # Extract argument names from the argument list using simple heuristics:

    # a) Remove the following expressions from the argument list:
    # - <...>
    # - [...]
    # - )(...)

    $fargs =~ s/\s*(?:$RE_ANGLES|$RE_BRACKETS|\)\s*$RE_PARENS)\s*//sg;

    # b) Keep all the names preceding a comma
    
    $fargs =~ s/\s*,\s*/,/g;
    my(@args) = ($fargs =~ /(\w+),/g);

    $EXTRA_DEBUG and print "DEBUG: arg list = (@args)\n";

    # Determine the actual function type and add a return documentation
    # if needed.

    $ftype = canonical_function_type($ftype);

    $EXTRA_DEBUG and print "DEBUG: actual ftype = $ftype\n";

    # Build the argument documentation

    my $doc = "";
    foreach my $arg (@args) {
        $doc .= "$indent * \\param[in] $arg TODOC\n";
    }

    if( $ftype ne "" and $ftype ne "void" ) {
        $doc .= "$indent * \\return TODOC\n";
    }

    # Return the documented function declaration

    return "$newlines$indent/**
$indent * \\brief TODOC
$indent * \\details TODOC
$doc$indent */
$indent$text";
}

#!
# \brief Fix all doxygen comments
# \param[in] file path to the input file
# \param[in] text the input text
# \return the input text with fixed comments
# \see fix_doxygen_comment()
#
sub fix_comments {
    my($file, $text) = @_;

    my $fix_spaces = $OPTIONS{fix_spaces};

    $text =~ s/\/\*[\*!]\s+([^*]|\*+[^*\/])*\*+\// fix_doxygen_comment($&, $fix_spaces) /sge;
    
    return $text;
}

#!
# \brief Fix a doxygen comment
# \param[in] comment a doxygen comment
# \param[in] fix_spaces agressively removes all leading spaces
# \return fixed documentation
#
sub fix_doxygen_comment {
    my($comment, $fix_spaces) = @_;

    $EXTRA_DEBUG and print "Fixing comment [[$comment]]\n";

    # Replace /*! by /**
    $comment =~ s/^\/\*!/\/\*\*/s;

    # Change doxygen directives to \directive
    $comment =~ s/\@(\w+)\b/\\$1/sg;

    # Capitalize word after \brief, \details, \note, \remark
    $comment =~ s/(\\(?:brief|details|note|remark)\s+)(\w)/$1\u$2/sg;

    # Remove spaces between \param and [
    $comment =~ s/(\\param)\h+\[/$1\[/g;

    # Add missing role after \param
    $comment =~ s/(\\param)\h+/$1\[in\] /g;

    # Remove extra spaces in param declaration
    $comment =~ s/(\\param\[.+?\])\h+(\w+)\h+/$1 $2 /g;

    # Remove extra spaces after doxygen directives
    $comment =~ s/(\\\w+)\h+/$1 /sg;

    # Remove extra leading spaces in comments
    # Preserve indentation in code sections
    # TODO: try to preserve list indentation

    my @lines = split(/^/, $comment);
    my $code_indent;
    my $in_code;

    foreach (@lines) {
        # Fix spaces before doxygen directives
        # Note: only realign doxygen directives of 2 letters and more.
        # Single letter directives (\p, \e, \c, ...) found at the beginning of
        # a line are the continuation of the previous line.
        s/^(\h+\*)\h*(\\\w{2,})\h+/$1 $2 /;

        if( $in_code ) {
            if( /\\endcode\b/ ) {
                $in_code = 0;
                undef $code_indent;
            } else {
                if( not defined($code_indent) ) {
                    ($code_indent) = (/^\h+\*(\h+)/);
                    if( not defined($code_indent) ) {
                        $code_indent = " ";
                    }
                }
                s/^(\h+\*)$code_indent/$1 /;
            }
        } else {
            if( /\\code\b/ ) {
                $in_code = 1;
            } else {
                if( $fix_spaces ) {
                    s/^(\h+\*)\h{2,}/$1  /;
                }
            }
        }
    }

    $comment = join('', @lines);

    return $comment;
}

#!
# \brief Matches a well balanced expression
# \param[in] input the pointer to the character before the opening char
# \param[in] open_char the opening char
# \param[in] close_char the close char to search
# \param[out] contents_ref is set to the tesxt of the full expression
# \return a pointer to the closing char
#
sub find_closing_char {
    my($input, $open_char, $close_char, $contents_ref) = @_;

    my $re = balanced_expression_regexp($open_char, $close_char);
    if( $input =~ $re ) {
        if( $contents_ref ) {
            $$contents_ref = $&;
        }
        return $';
    }

    return undef;
}


#!
# \brief Creates a regex to matche well balanced expressions
# \param[in] open_char the opening char
# \param[in] close_char the close char to search
# \return the regular expression
#
sub balanced_expression_regexp {
    my($open_char, $close_char) = @_;

    return qr/(
        $open_char
        (?:
            # Preprocessor directive
            \#(?:\\\n|\/\*.*?\*\/|\N)*

            # C++ comment
            |\/\/\N*(?:\n\s*\/\/\N*)*

            # C comment
            |\/\*.*?\*\/

            # String constant
            |"(?:\\.|\N)*?"

            # Character constants
            |'(?:\\.|[^\\'])+'

            # Not close char or open char
            |[^$open_char$close_char]

            # Or the recursive pattern
            |(?-1)
        )*
        $close_char
    )/sx;
}

