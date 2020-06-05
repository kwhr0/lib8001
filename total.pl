#! /usr/bin/perl
while (<>) {
	$t += hex($1) if /(\w+)\s+l__(\w+)/ && $2 ne 'HEAP';
}
printf "%d bytes (%.0f%%)\n", $t, 100 * $t / 0xe700;
exit 0;
