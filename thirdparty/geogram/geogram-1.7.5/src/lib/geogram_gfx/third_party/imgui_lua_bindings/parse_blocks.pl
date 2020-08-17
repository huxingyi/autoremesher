##!/usr/bin/perl
#use strict;
#use warnings;
#use diagnostics;
#
#my ($blocksref, $blocknamesref) = parse_blocks();
#
#my @blocks = @$blocksref;
#my @blocknames = @$blocknamesref;
#
#my $name;
#my $block;
#foreach $name (@blocknames) {
#  print "BLOCKNAME" . $name;
#}
#
#foreach $block (@blocks) {
#  print "---------------BLOCK START-----------\n";
#  print $block;
#  print "---------------BLOCK END -------------\n"
#}

#returns list of string blocks from stdin
sub parse_blocks {
  my @blocks;
  my @blocknames;
  my $lastline;
  my $curBlock;
  while (my $line = <STDIN>) {
   #This is a crappy way of parsing that works for
   # imgui
   if ($line =~ m/^{$/) {
     push @blocknames, $lastline;
     $curBlock = "";
     next;
   }
   if ($line =~ m/^};$|^} \/\//) {
     push @blocks, $curBlock;
     $curBlock = "";
     next;
   }

   $curBlock .= $line . "\n";
   $lastline = $line;
  }
  return (\@blocks, \@blocknames);
}

1;
