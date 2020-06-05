#! /usr/bin/perl

@ARGV || die "Usage: mkadr.pl <sym files>\n";

foreach (@ARGV) {
	next unless /(\w+)\.sym/;
	$m = $1;
	$path = $_;
	$path =~ s/sym$/c/;
	$src{$m} = $m.".c" if -f $path;
	$path = $_;
	$path =~ s/sym$/s/;
	$src{$m} = $m.".s" if -f $path;
	open(S, $_) || die;
	while (<S>) {
		if (/(\d)\s_(\w+)\s+([0-9A-F]{4})/) {
			if ($1) {
				$data{$m."/".$2} = 1;
			}
			else {
				$sym{$m."/".$2} = hex($3);
			}
		}
	}
	close S;
}

open(S, <*.map>) || die;
while (<S>) {
	$map{$3."/".$2} = hex($1) if /([0-9A-F]{4})\s+_(\w+)\s+(\w*)/;
	$lib{$1} = 1 if /z80\.lib\W+(\w+)\.rel/;
}
close S;

foreach (sort keys %map) {
	$abs{$map{$_}} = $_;
	next unless /(\w+)\//;
	if (defined($sym{$_})) {
		$ofs{$1} = $map{$_} - $sym{$_};
	}
	elsif (!$data{$_} && !$lib{$1}) {
		print STDERR "Undefined: $_\n";
	}
}

foreach (keys %sym) {
	$abs{$sym{$_} + $ofs{$1}} = $_ if /(\w+)\// && defined($ofs{$1});
}

foreach (sort { $a <=> $b } keys %abs) {
	$m = $abs{$_};
	$m =~ s/\/.*//;
	$f = $abs{$_};
	$f =~ s/.*\///;
	printf "%04X %s (%s)\n", $_, $f, $src{$m} ? $src{$m} : "SDCC";
}
exit 0;
