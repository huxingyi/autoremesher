#!/usr/bin/perl
use strict;
use warnings;
use diagnostics;
# This works for IMGUI 1.50 WIP and does not get all functions
#
# to use ./generate_imgui_bindings.pl <imgui.h >imgui_iterator.cpp
# and define macros properly as in example imgui_lua_bindings.cpp
#
# check imgui_iterator for explanations of why some functions are not supported yet

#TODO switch to export style with modules or just copy paste
require "./parse_blocks.pl";

sub generateNamespaceImgui {
  my ($imguiBlock) = @_;


#define bannedNames with keys of functions to exclude them
# EXAMPLE:
  my %bannedNames = (
    "NewFrame" => "banned",
    "Render" => "banned",
    "Shutdown" => "banned" );
#
# This is only useful for ENABLE_IM_LUA_END_STACK
# We hold a list of differnet 'things' that can be pushed to the stack
# i.e. Group for BeginGroup
# It usually works like this BeginBlah EndBlah

# We have to redefine stuff when it doesn't work so cleanly
  my %beginN = (
    "TreeNode" => "Tree",
    "TreePush" => "Tree",
    "PushStyleVar" => "StyleVar"
    );
  my %changeN = (
    "Tree" => "TreePop",
    "StyleVar"=> "PopStyleVar"
    );
  my %endN = (
    "TreePop" => "Tree",
    "PopStyleVar" => "StyleVar"
    );
  my %endOverride = (
    "PopupModal" => "Popup",
    "PopupContextItem" => "Popup",
    "PopupContextWindow" => "Popup",
    "PopupContextVoid" => "Popup" );


  my $numSupported = 0;
  my $numUnsupported = 0;
  my $line;
  my %funcNames;
  my %endTypeToInt;
  my @endTypes;
  foreach $line (split /\n/, $imguiBlock) {
    #replace ImVec2(x, y) with ImVec2 x, y so it's easier for regex (and ImVec4)
    $line =~ s/ImVec2\(([^,]*),([^\)]*)\)/ImVec2 $1 $2/g;
    $line =~ s/ImVec4\(([^,]*),([^\)]*),([^\)]*),([^\)]*)\)/ImVec4 $1 $2 $3 $4/g;

    #delete this so it's eaiser for regexes
    $line =~ s/ IM_PRINTFARGS\(.\);/;/g;
    if ($line =~ m/ *IMGUI_API *([^ ]+) *([^\(]+)\(([^\;]*)\);/) {
      print "//" . $line . "\n";
      # this will be set to 0 if something is not supported yet
      my $shouldPrint = 1;
      my @args = split(',', $3);
      # things to do before calling real c++ function
      my @before;
      # arguments to real c++ function
      my @funcArgs;
      # things to do after callign real c++ function
      my @after;
      # real c++ function name
      my $funcName = $2;
      if (defined($bannedNames{$funcName})) {
        print "//Not allowed to use this function\n";
        $shouldPrint = 0;
      }
      # c++ type of return value
      my $retType;
      # macro used for calling function
      my $callMacro;
      # if it has a return value (yes I know this is not the cleanest code)
      my $hasRet = 1;
      if ($1 =~ /^void$/) {
        $callMacro = "CALL_FUNCTION_NO_RET";
        $hasRet = 0;
      } elsif ($1 =~ /^bool$/) {
        $callMacro = "CALL_FUNCTION";
        push(@funcArgs, "bool");
        push(@after, "PUSH_BOOL(ret)");
      } elsif ($1 =~ /^float$/) {
        $callMacro = "CALL_FUNCTION";
        push(@funcArgs, "float");
        push(@after, "PUSH_NUMBER(ret)");
      } elsif ($1 =~ /^ImVec2$/) {
        $callMacro = "CALL_FUNCTION";
        push(@funcArgs, "ImVec2");
        push(@after, "PUSH_NUMBER(ret.x)");
        push(@after, "PUSH_NUMBER(ret.y)");
      } elsif ($1 =~ /^(unsigned int|ImGuiID|ImU32)$/) {
        $callMacro = "CALL_FUNCTION";
        push(@funcArgs, "unsigned int");
        push(@after, "PUSH_NUMBER(ret)");
      } else {
        print "// Unsupported return type $1\n";
        $shouldPrint = 0;
      }
      for (my $i = 0; $i < @args; $i++) {
        # bool * x = NULL or bool * x
        if ($args[$i] =~ m/^ *bool *\* *([^ =\[]*)( = NULL|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ = NULL$/) {
            push(@before, "OPTIONAL_BOOL_POINTER_ARG($name)");
          } else {
            push(@before, "BOOL_POINTER_ARG($name)");
          }
          push(@funcArgs, $name);
          push(@after, "END_BOOL_POINTER($name)");
        # float * x
        } elsif ($args[$i] =~ m/^ *float *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "FLOAT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_FLOAT_POINTER($name)");
          #float a or float a = number
        } elsif ($args[$i] =~ m/^ *float *([^ =\[]*)( *= *[^ ]*|)$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *([^ ]*)$/) {
            push(@before, "OPTIONAL_NUMBER_ARG($name, $1)");
          } else {
            push(@before, "NUMBER_ARG($name)");
          }
          push(@funcArgs, $name);
          # const char* a or const char* a = NULL or "blah"
        } elsif ($args[$i] =~ m/^ *const char\* *([^ =\[]*)( *= *(NULL|".*")|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *NULL$/) {
            push(@before, "OPTIONAL_LABEL_ARG($name)");
          } else {
            push(@before, "LABEL_ARG($name)");
          }
          push(@funcArgs, $name);
        #const ImVec2& with default or not
        } elsif ($args[$i] =~ m/^ *const ImVec2& ([^ ]*) *(= * ImVec2 [^ ]* [^ ]*|) *$/) {
          my $name = $1;
          if ($2 =~ m/^= * ImVec2 ([^ ]*) ([^ ]*)$/) {
            push(@before, "OPTIONAL_IM_VEC_2_ARG($name, $1, $2)");
          } else {
            push(@before, "IM_VEC_2_ARG($name)");
          }
          push(@funcArgs, $name);
        # ImVec2 
        } elsif ($args[$i] =~ m/^ *ImVec2 ([^ ]*) *$/) {
          my $name = $1;
          push(@before, "IM_VEC_2_ARG($name)");
          push(@funcArgs, $name);
        #const ImVec4& with default or not
        } elsif ($args[$i] =~ m/^ *const ImVec4& ([^ ]*) *(= * ImVec4 [^ ]* [^ ]* [^ ]* [^ ]*|) *$/) {
          my $name = $1;
          if ($2 =~ m/^= * ImVec4 ([^ ]*) ([^ ]*) ([^ ]*) ([^ ]*)$/) {
            push(@before, "OPTIONAL_IM_VEC_4_ARG($name, $1, $2, $3, $4)");
          } else {
            push(@before, "IM_VEC_4_ARG($name)");
          }
          push(@funcArgs, $name);
          # one of the various enums
          # we are handling these as ints
        } elsif ($args[$i] =~ m/^ *(ImGuiWindowFlags|ImGuiCol|ImGuiStyleVar|ImGuiKey|ImGuiAlign|ImGuiColorEditMode|ImGuiMouseCursor|ImGuiSetCond|ImGuiInputTextFlags|ImGuiSelectableFlags) ([^ ]*)( = 0|) *$/) {
         #These are ints
         my $name = $2;
          if ($3 =~ m/^ = 0$/) {
            push(@before, "OPTIONAL_INT_ARG($name, 0)");
          } else {
            push(@before, "INT_ARG($name)");
          }
          push(@funcArgs, $name);
          #int with default value or not
        } elsif ($args[$i] =~ m/^ *int ([^ =\[]*)( = [^ ]*|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ = ([^ ]*)$/) {
            push(@before, "OPTIONAL_INT_ARG($name, $1)");
          } else {
            push(@before, "INT_ARG($name)");
          }
          push(@funcArgs, $name);
        #unsigned int with default value or not
        } elsif ($args[$i] =~ m/^ *(unsigned +int|ImGuiID|ImU32) ([^ =\[]*)( = [^ ]*|) *$/) {
          my $name = $2;
          if ($2 =~ m/^ = ([^ ]*)$/) {
            push(@before, "OPTIONAL_UINT_ARG($name, $1)");
          } else {
            push(@before, "UINT_ARG($name)");
          }
          push(@funcArgs, $name);
        #ImTextureID or const ImTextureID&
        # const ImTextureID& is the same thing as var
        # as lua is concerned
        } elsif ($args[$i] =~ m/^ *(ImTextureID|const ImTextureID&) ([^ =\[]*) *$/) {
          my $name = $2;
          push(@before, "IM_TEXTURE_ID_ARG($name)");
          push(@funcArgs, $name);
          # bool with default value or not
        } elsif ($args[$i] =~ m/^ *bool ([^ =\[]*)( *= *true| *= *false|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *([^ ]*)$/) {
            push(@before, "OPTIONAL_BOOL_ARG($name, $1)");
          } else {
            push(@before, "BOOL_ARG($name)");
          }
          push(@funcArgs, $name);
        # int * x
        } elsif ($args[$i] =~ m/^ *int *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "INT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_INT_POINTER($name)");
        # unsigned int * x
        } elsif ($args[$i] =~ m/^ *unsigned +int *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "UINT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_UINT_POINTER($name)");
          # we don't support variadic functions yet but we let you use it without extra variables
        } elsif ($args[$i] =~ m/^ *\.\.\. *$/) {
          print "// Variadic functions aren't suppported but here it is anyway\n";
        } else {
          print "// Unsupported arg type " . $args[$i] . "\n";
          $shouldPrint = 0;
        }
      }
      if ($shouldPrint != 0) {
        my $luaFunc = $funcName;
        # Stupid way of implementing overriding
        while($funcNames{$luaFunc}) {
          $luaFunc .= "_" . scalar(@args);
        }
        $funcNames{$luaFunc} = 1;

        print "IMGUI_FUNCTION($luaFunc)\n";
        for (my $i = 0; $i < @before; $i++) {
          print $before[$i] . "\n";
        }

        print $callMacro . "($funcName";
        for (my $i = 0; $i < @funcArgs; $i++) {
          print ", " . $funcArgs[$i];
        }
        print ")\n";

        #for begin and end stack stuff
        if ($funcName =~ m/^Begin(.*)$/ || defined($beginN{$funcName})) {
          my $curEndType;
          if (defined($beginN{$funcName})) {
            $curEndType = $beginN{$funcName};
          } else {
            $curEndType = $1;
          }
          if (defined($endOverride{$curEndType})) {
            $curEndType = $endOverride{$curEndType};
          }
          if (!defined($endTypeToInt{$curEndType})) {
            $endTypeToInt{$curEndType} = scalar(@endTypes);
            push(@endTypes, $curEndType);
          }
          my $curEndTypeInt = $endTypeToInt{$curEndType};
          if ($hasRet) {
            print "IF_RET_ADD_END_STACK($curEndTypeInt)\n";
          } else {
            print "ADD_END_STACK($curEndTypeInt)\n";
          }
        } elsif ($funcName =~ m/^End(.*)$/ || defined($endN{$funcName})) {
          my $curEndType;
          if (defined($endN{$funcName})) {
            $curEndType = $endN{$funcName};
          } else {
            $curEndType = $1;
          }
          if (defined($endOverride{$curEndType})) {
            $curEndType = $endOverride{$curEndType};
          }
          if (!defined($endTypeToInt{$curEndType})) {
            $endTypeToInt{$curEndType} = scalar(@endTypes);
            push(@endTypes, $curEndType);
          }
          my $curEndTypeInt = $endTypeToInt{$curEndType};
          print "POP_END_STACK($curEndTypeInt)\n"
        }

        for (my $i = 0; $i < @after; $i++) {
          print $after[$i] . "\n";
        }
        print "END_IMGUI_FUNC\n";
        $numSupported += 1;
      } else {
        $numUnsupported += 1;
      }
    } elsif ($line =~ m/^} \/\/ namespace ImGui$/) {
      last;
    }
  }
#for end stack stuff
  print "END_STACK_START\n";
  for (my $i = 0; $i < @endTypes; $i++) {
    my $endFunc;
    if (defined($changeN{$endTypes[$i]})) {
      $endFunc = $changeN{$endTypes[$i]};
    } else {
      $endFunc = "End" . $endTypes[$i];
    }
    print "END_STACK_OPTION($i, " . $endFunc .")\n";
  }
  print "END_STACK_END\n";

#debug info
  print STDERR "Supported: $numSupported Unsupported: $numUnsupported\n";

}

# TODO Combine shared logic of draw list and main imgui
sub generateDrawListFunctions {
  my ($drawListBlock) = @_;


#define bannedNames with keys of functions to exclude them
# EXAMPLE:
  my %bannedNames = (
   );
#
# This is only useful for ENABLE_IM_LUA_END_STACK
# We hold a list of differnet 'things' that can be pushed to the stack
# i.e. Group for BeginGroup
# It usually works like this BeginBlah EndBlah

# We have to redefine stuff when it doesn't work so cleanly
  my %beginN = (
    );
  my %changeN = (
    );
  my %endN = (
    );
  my %endOverride = (
     );


  my $numSupported = 0;
  my $numUnsupported = 0;
  my $line;
  my %funcNames;
  my %endTypeToInt;
  my @endTypes;
  foreach $line (split /\n/, $drawListBlock) {
    #replace ImVec2(x, y) with ImVec2 x, y so it's easier for regex (and ImVec4)
    $line =~ s/ImVec2\(([^,]*),([^\)]*)\)/ImVec2 $1 $2/g;
    $line =~ s/ImVec4\(([^,]*),([^\)]*),([^\)]*),([^\)]*)\)/ImVec4 $1 $2 $3 $4/g;

    #delete this so it's eaiser for regexes
    $line =~ s/ IM_PRINTFARGS\(.\);/;/g;
    if ($line =~ m/ *(IMGUI_API|inline) *([^ ]+) *([^\(]+)\(([^\;]*)\);/) {
      print "//" . $line . "\n";
      # this will be set to 0 if something is not supported yet
      my $shouldPrint = 1;
      my @args = split(',', $4);
      # things to do before calling real c++ function
      my @before;
      # arguments to real c++ function
      my @funcArgs;
      # things to do after callign real c++ function
      my @after;
      # real c++ function name
      my $funcName = $3;
      if (defined($bannedNames{$funcName})) {
        print "//Not allowed to use this function\n";
        $shouldPrint = 0;
      }
      # c++ type of return value
      my $retType;
      # macro used for calling function
      my $callMacro;
      # if it has a return value (yes I know this is not the cleanest code)
      my $hasRet = 1;
      if ($2 =~ /^void$/) {
        $callMacro = "DRAW_LIST_CALL_FUNCTION_NO_RET";
        $hasRet = 0;
      } elsif ($2 =~ /^bool$/) {
        $callMacro = "DRAW_LIST_CALL_FUNCTION";
        push(@funcArgs, "bool");
        push(@after, "PUSH_BOOL(ret)");
      } elsif ($2 =~ /^float$/) {
        $callMacro = "DRAW_LIST_CALL_FUNCTION";
        push(@funcArgs, "float");
        push(@after, "PUSH_NUMBER(ret)");
      } elsif ($2 =~ /^ImVec2$/) {
        $callMacro = "DRAW_LIST_CALL_FUNCTION";
        push(@funcArgs, "ImVec2");
        push(@after, "PUSH_NUMBER(ret.x)");
        push(@after, "PUSH_NUMBER(ret.y)");
      } elsif ($2 =~ /^(unsigned int|ImGuiID|ImU32)$/) {
        $callMacro = "DRAW_LIST_CALL_FUNCTION";
        push(@funcArgs, "unsigned int");
        push(@after, "PUSH_NUMBER(ret)");
      } else {
        print "// Unsupported return type $1\n";
        $shouldPrint = 0;
      }
      for (my $i = 0; $i < @args; $i++) {
        # bool * x = NULL or bool * x
        if ($args[$i] =~ m/^ *bool *\* *([^ =\[]*)( = NULL|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ = NULL$/) {
            push(@before, "OPTIONAL_BOOL_POINTER_ARG($name)");
          } else {
            push(@before, "BOOL_POINTER_ARG($name)");
          }
          push(@funcArgs, $name);
          push(@after, "END_BOOL_POINTER($name)");
        # float * x
        } elsif ($args[$i] =~ m/^ *float *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "FLOAT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_FLOAT_POINTER($name)");
          #float a or float a = number
        } elsif ($args[$i] =~ m/^ *float *([^ =\[]*)( *= *[^ ]*|)$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *([^ ]*)$/) {
            push(@before, "OPTIONAL_NUMBER_ARG($name, $1)");
          } else {
            push(@before, "NUMBER_ARG($name)");
          }
          push(@funcArgs, $name);
          # const char* a or const char* a = NULL or "blah"
        } elsif ($args[$i] =~ m/^ *const char\* *([^ =\[]*)( *= *(NULL|".*")|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *NULL$/) {
            push(@before, "OPTIONAL_LABEL_ARG($name)");
          } else {
            push(@before, "LABEL_ARG($name)");
          }
          push(@funcArgs, $name);
        # ImVec2 
        } elsif ($args[$i] =~ m/^ *ImVec2 ([^ ]*) *$/) {
          my $name = $1;
          push(@before, "IM_VEC_2_ARG($name)");
          push(@funcArgs, $name);
        #const ImVec2& with default or not
        } elsif ($args[$i] =~ m/^ *const ImVec2& ([^ ]*) *(= * ImVec2 [^ ]* [^ ]*|) *$/) {
           my $name = $1;
          if ($2 =~ m/^= * ImVec2 ([^ ]*) ([^ ]*)$/) {
            push(@before, "OPTIONAL_IM_VEC_2_ARG($name, $1, $2)");
          } else {
            push(@before, "IM_VEC_2_ARG($name)");
          }
          push(@funcArgs, $name);
        #const ImVec4& with default or not
        } elsif ($args[$i] =~ m/^ *const ImVec4& ([^ ]*) *(= * ImVec4 [^ ]* [^ ]* [^ ]* [^ ]*|) *$/) {
          my $name = $1;
          if ($2 =~ m/^= * ImVec4 ([^ ]*) ([^ ]*) ([^ ]*) ([^ ]*)$/) {
            push(@before, "OPTIONAL_IM_VEC_4_ARG($name, $1, $2, $3, $4)");
          } else {
            push(@before, "IM_VEC_4_ARG($name)");
          }
          push(@funcArgs, $name);
          # one of the various enums
          # we are handling these as ints
        } elsif ($args[$i] =~ m/^ *(ImGuiWindowFlags|ImGuiCol|ImGuiStyleVar|ImGuiKey|ImGuiAlign|ImGuiColorEditMode|ImGuiMouseCursor|ImGuiSetCond|ImGuiInputTextFlags|ImGuiSelectableFlags) ([^ ]*)( = 0|) *$/) {
         #These are ints
         my $name = $2;
          if ($3 =~ m/^ = 0$/) {
            push(@before, "OPTIONAL_INT_ARG($name, 0)");
          } else {
            push(@before, "INT_ARG($name)");
          }
          push(@funcArgs, $name);
          #int with default value or not
        } elsif ($args[$i] =~ m/^ *int ([^ =\[]*)( = [^ ]*|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ = ([^ ]*)$/) {
            push(@before, "OPTIONAL_INT_ARG($name, $1)");
          } else {
            push(@before, "INT_ARG($name)");
          }
          push(@funcArgs, $name);
        #unsigned int with default value or not
        } elsif ($args[$i] =~ m/^ *(unsigned +int|ImGuiID|ImU32) ([^ =\[]*)( = [^ ]*|) *$/) {
          my $name = $2;
          if ($2 =~ m/^ = ([^ ]*)$/) {
            push(@before, "OPTIONAL_UINT_ARG($name, $1)");
          } else {
            push(@before, "UINT_ARG($name)");
          }
          push(@funcArgs, $name);
        #ImTextureID or const ImTextureID&
        # const ImTextureID& is the same thing as var
        # as lua is concerned
        } elsif ($args[$i] =~ m/^ *(ImTextureID|const ImTextureID&) ([^ =\[]*) *$/) {
          my $name = $2;
          push(@before, "IM_TEXTURE_ID_ARG($name)");
          push(@funcArgs, $name);
          # bool with default value or not
        } elsif ($args[$i] =~ m/^ *bool ([^ =\[]*)( *= *true| *= *false|) *$/) {
          my $name = $1;
          if ($2 =~ m/^ *= *([^ ]*)$/) {
            push(@before, "OPTIONAL_BOOL_ARG($name, $1)");
          } else {
            push(@before, "BOOL_ARG($name)");
          }
          push(@funcArgs, $name);
        # int * x
        } elsif ($args[$i] =~ m/^ *int *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "INT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_INT_POINTER($name)");
        # unsigned int * x
        } elsif ($args[$i] =~ m/^ *unsigned +int *\* *([^ =\[]*)$/) {
          my $name = $1;
          push(@before, "UINT_POINTER_ARG($name)");
          push(@funcArgs, $name);
          push(@after, "END_UINT_POINTER($name)");
          # we don't support variadic functions yet but we let you use it without extra variables
        } elsif ($args[$i] =~ m/^ *\.\.\. *$/) {
          print "// Variadic functions aren't suppported but here it is anyway\n";
        } else {
          print "// Unsupported arg type " . $args[$i] . "\n";
          $shouldPrint = 0;
        }
      }
      if ($shouldPrint != 0) {
        my $luaFunc = $funcName;
        # Stupid way of implementing overriding
        while($funcNames{$luaFunc}) {
          $luaFunc .= "_" . scalar(@args);
        }
        $funcNames{$luaFunc} = 1;

        print "IMGUI_FUNCTION_DRAW_LIST($luaFunc)\n";
        for (my $i = 0; $i < @before; $i++) {
          print $before[$i] . "\n";
        }

        print $callMacro . "($funcName";
        for (my $i = 0; $i < @funcArgs; $i++) {
          print ", " . $funcArgs[$i];
        }
        print ")\n";

        #for begin and end stack stuff
        if ($funcName =~ m/^Begin(.*)$/ || defined($beginN{$funcName})) {
          my $curEndType;
          if (defined($beginN{$funcName})) {
            $curEndType = $beginN{$funcName};
          } else {
            $curEndType = $1;
          }
          if (defined($endOverride{$curEndType})) {
            $curEndType = $endOverride{$curEndType};
          }
          if (!defined($endTypeToInt{$curEndType})) {
            $endTypeToInt{$curEndType} = scalar(@endTypes);
            push(@endTypes, $curEndType);
          }
          my $curEndTypeInt = $endTypeToInt{$curEndType};
          if ($hasRet) {
            print "IF_RET_ADD_END_STACK($curEndTypeInt)\n";
          } else {
            print "ADD_END_STACK($curEndTypeInt)\n";
          }
        } elsif ($funcName =~ m/^End(.*)$/ || defined($endN{$funcName})) {
          my $curEndType;
          if (defined($endN{$funcName})) {
            $curEndType = $endN{$funcName};
          } else {
            $curEndType = $1;
          }
          if (defined($endOverride{$curEndType})) {
            $curEndType = $endOverride{$curEndType};
          }
          if (!defined($endTypeToInt{$curEndType})) {
            $endTypeToInt{$curEndType} = scalar(@endTypes);
            push(@endTypes, $curEndType);
          }
          my $curEndTypeInt = $endTypeToInt{$curEndType};
          print "POP_END_STACK($curEndTypeInt)\n"
        }

        for (my $i = 0; $i < @after; $i++) {
          print $after[$i] . "\n";
        }
        print "END_IMGUI_FUNC\n";
        $numSupported += 1;
      } else {
        $numUnsupported += 1;
      }
    }
  }
#for end stack stuff
  #print "END_STACK_START\n";
  #for (my $i = 0; $i < @endTypes; $i++) {
  #  my $endFunc;
  #  if (defined($changeN{$endTypes[$i]})) {
  #    $endFunc = $changeN{$endTypes[$i]};
  #  } else {
  #    $endFunc = "End" . $endTypes[$i];
  #  }
  #  print "END_STACK_OPTION($i, " . $endFunc .")\n";
  #}
  #print "END_STACK_END\n";

#debug info
  print STDERR "Supported: $numSupported Unsupported: $numUnsupported\n";

}


my ($blocksref, $blocknamesref) = parse_blocks();

my @blocks = @$blocksref;
my @blocknames = @$blocknamesref;

for (my $i=0; $i < scalar @blocks; $i++) {
  print "//" . $blocknames[$i] . "\n";
  if ($blocknames[$i] eq "namespace ImGui\n") {
    generateNamespaceImgui($blocks[$i]);
  }
  if ($blocknames[$i] eq "struct ImDrawList\n") {
    generateDrawListFunctions($blocks[$i]);
  }
}


