#!/usr/bin/perl

=pod
This script will install dependencies for OpenViBE Designer.

The installer uses the native package manager.

Currently supported Linux Distributions are:
- Ubuntu 14.04 LTS
- Ubuntu 16.04 LTS
=cut

use strict;
use English;
use FindBin;

sub usage {
  print "$0 [-h][-y]\n";
  print "Install OpenVibe build dependencies\n";
  print "    Options:\n";
  print "      -h: this help\n";
  print "      -y: assume 'yes' to all prompts. Make it possible to run non-interactively.\n";
};


if ($#ARGV > 0) {
  usage();
  exit(1);
}

my $assume_yes = 0;

if ($#ARGV == 0) {
  if ($ARGV[0] eq "-h") {
    usage();
    exit(0);
  } elsif ($ARGV[0] eq "-y") {
     $assume_yes = 1;
  } else {
    usage();
    exit(1);
  }
}

# Check for the release version and set the update and install commands

my $distribution = 'Unknown';
my $update_packages_command = '';
my $package_install_command = '';
my $add_repository_command  = '';

my $lsb_distributor = `lsb_release --id --short`;
my $lsb_release = `lsb_release --release --short`;

if ($lsb_distributor =~ 'Ubuntu') {
  $update_packages_command = 'sudo apt-get update';
  if ($assume_yes) {
    $package_install_command = 'sudo apt-get -y install';
    $add_repository_command  = 'sudo add-apt-repository -y universe';
  } else {
    $package_install_command = 'sudo apt-get install';
    $add_repository_command  = 'sudo add-apt-repository universe';
  }
  if ($lsb_release =~ '14.04') {
    $distribution = 'Ubuntu 14.04';
  } elsif ($lsb_release =~ '16.04') {
    $distribution = 'Ubuntu 16.04';
  }
}

$distribution eq 'Unknown' and die('This distribution is unsupported');

print "Installing dependencies for: $distribution\n";

# Create the list of packages to install

my @packages = ();

if ($distribution eq 'Ubuntu 14.04') {
  push @packages, "libgtk2.0-dev";
  push @packages, "libglade2-dev";
} elsif ($distribution eq 'Ubuntu 16.04') {
  push @packages, "libgtk2.0-dev";
  push @packages, "libglade2-dev";
}

# Update package list
print "Updating package database...\n";
system($add_repository_command);
($CHILD_ERROR != 0) and die('Failed to add additional repositories');
system($update_packages_command);
($CHILD_ERROR != 0) and die('Failed to update the package databases');

# Install the packages
print "Will install following packages:\n";
print (join ' ', @packages), "\n";

system("$package_install_command " . (join ' ', @packages));
($CHILD_ERROR != 0) and die('Failed to install the required packages');

print("OpenViBE dependencies were successfully installed\n");
