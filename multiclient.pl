#!/usr/bin/perl

$numclients = 10;
$numclients = $ARGV[0] if ($ARGV[0]);

$config = "ldapbenchmark.conf";
$results = "./results.txt";
$total_queries = 0;

system("date"); print "\n";

for ($i=1; $i <= $numclients ;$i++) {
	print "Starting client $i.\n";
	system("./ldapbenchmark config/$config | grep Process | awk '{print \"Client $i - \" \$8 \" q/s\"}' | tee -a $results &");
}

print "\nWaiting for clients to finish... Please be patient.\n\n";
for (;;) {
	sleep 2;
	$done = `grep -c 'Client $numclients' $results`;
	if ($done == 1) {
		open(IN,"$results");
		@IN=<IN>;
		close(IN);

		foreach $line (@IN) {
			chop $line;
			($n,$n,$n,$Q) = split(/ /,"$line");
			$total_queries += $Q;

		}
		$total_queries = sprintf("%.2f",$total_queries);

		print "----------------------\n";
		print "Total Queries: $total_queries q/s\n\n";

		unlink("$results");
		exit(0);
	}
}	
