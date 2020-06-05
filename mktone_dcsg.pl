#! /usr/bin/perl
# This table is also used by host C, should not be assembler.

print "extern const unsigned short tonetable_dcsg[] = {\n";
for ($i = 12; $i < 128; $i++) {
	print "\t" unless $i % 12;
	$d = 44100 / 2 / (440.0 * 2.0 ** (($i - 69) / 12.0));
	$d = 0 if $d > 1023;
	printf "%d,%s", $d, $i % 12 < 11 ? " " : "\n";
}
print "\n};\n";
exit 0;
