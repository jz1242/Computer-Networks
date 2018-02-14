h1 rm -rf measurements
h1 mkdir measurements

h1 echo "\h1, \h3 - Testing RTT of links with ping..."
h1 ping h3 -c 20 | tail -n 2 >> ./measurements/latency_L1.txt
h3 echo "\h3, \h4 - Testing RTT of links with ping..."
h3 ping h4 -c 20 | tail -n 2 >> ./measurements/latency_L2.txt
h3 echo "\h3, \h7 - Testing RTT of links with ping..."
h3 ping h7 -c 20 | tail -n 2 >> ./measurements/latency_L3.txt
h4 echo "\h4, \h8 - Testing RTT of links with ping..."
h4 ping h8 -c 20 | tail -n 2 >> ./measurements/latency_L4.txt
h8 echo "\h8, \h9 - Testing RTT of links with ping..."
h8 ping h9 -c 20 | tail -n 2 >> ./measurements/latency_L5.txt

h1 echo "\h1, \h3 - Testing throughput of links with ./iPerfer..."
h1 ./iPerfer -s -p 2000 >> ./measurements/throughput_L1.txt &
h3 ./iPerfer -c -h 10.0.0.1 -p 2000 -t 20  >> ./measurements/throughput_L1.txt
h3 echo "\h3, \h4 - Testing throughput of links with ./iPerfer..."
h3 ./iPerfer -s -p 2001 >> ./measurements/throughput_L2.txt &
h4 ./iPerfer -c -h 10.0.0.3  -p 2001 -t 20 >> ./measurements/throughput_L2.txt
h3 echo "\h3, \h7 - Testing throughput of links with ./iPerfer..."
h3 ./iPerfer -s -p 2002 >> ./measurements/throughput_L3.txt &
h7 ./iPerfer -c  -h 10.0.0.3 -p 2002 -t 20 >> ./measurements/throughput_L3.txt
h4 echo "\h4, \h8 - Testing throughput of links with ./iPerfer..."
h4 ./iPerfer -s -p 2003 >> ./measurements/throughput_L4.txt &
h8 ./iPerfer -c -h 10.0.0.4 -p 2003 -t 20 >> ./measurements/throughput_L4.txt
h8 echo "\h8, \h9 - Testing throughput of links with ./iPerfer..."
h8 ./iPerfer -s -p 2004 >> ./measurements/throughput_L5.txt &
h9 ./iPerfer -c -h 10.0.0.8 -p 2004 -t 20 >> ./measurements/throughput_L5.txt

h1 wait 
h10 wait

h1 echo "\h1, \h10 - Testing RTT of links with ping..."
h1 ping h10 -c 20 | tail -n 2 >> ./measurements/latency_Q2.txt
h1 echo "\h1, \h10 - Testing throughput of links with ./iPerfer..."
h1 ./iPerfer -s -p 2005 > ./measurements/throughput_Q2.txt &
h10 ./iPerfer -c -h 10.0.0.1 -p 2005 -t 20 >> ./measurements/throughput_Q2.txt

h1 wait 
h10 wait

h1 echo "\h1 -> \h6, \h2 -> \h9 - 2 Pair Multiplexing RTT of links with ping..."
h1 ping h6 -c 20 | tail -n 2 &
h2 ping h9 -c 20 | tail -n 2 >> ./measurements/latency_Q3_1.txt

h1 wait 
h2 wait

h1 echo "\h1 -> \h6, \h2 -> \h9, \h5 -> \h10 - 3 Pair Multiplexing RTT of links with ping..."
h1 ping h6 -c 20 | tail -n 2 &
h2 ping h9 -c 20 | tail -n 2 &
h5 ping h10 -c 20 | tail -n 2 >> ./measurements/latency_Q3_2.txt

h1 wait 
h2 wait
h5 wait 

h1 echo "\h1 -> \h6, \h2 -> \h9 - 2 Pair Multiplexing throughput test from \s1 to \s6..."
h1 ./iPerfer -s -p 2006 &
h2 ./iPerfer -s -p 2007 >> ./measurements/throughput_Q3_1.txt &
h6 ./iPerfer -c -h 10.0.0.1 -p 2006 -t 20  &
h9 ./iPerfer -c -h 10.0.0.2 -p 2007 -t 20 >> ./measurements/throughput_Q3_1.txt 

h1 wait 
h2 wait 
h6 wait 
h9 wait

h1 echo "\h1 -> \h6, \h2 -> \h9, \h5 -> \h10 - 3 Pair Multiplexing throughput test from \s1 to \s6..."
h1 ./iPerfer -s -p 2008 &
h2 ./iPerfer -s -p 2009 &
h5 ./iPerfer -s -p 2010 >> ./measurements/throughput_Q3_2.txt &
h6 ./iPerfer -c -h 10.0.0.1 -p 2008 -t 20 &
h9 ./iPerfer -c -h 10.0.0.2 -p 2009 -t 20 &
h10 ./iPerfer -c -h 10.0.0.5 -p 2010 -t 20 >> ./measurements/throughput_Q3_2.txt 

h1 wait 
h2 wait 
h5 wait 
h6 wait 
h9 wait 
h10 wait

h1 echo "\h1 -> \h10, \h3 -> \h8 - 2 Pair Multiplexing RTT of middle links with ping..."
h1 ping h10 -c 20 | tail -n 2 >> ./measurements/latency_h1-h10.txt &
h3 ping h8 -c 20 | tail -n 2 >> ./measurements/latency_h3-h8.txt

h1 wait 
h3 wait

h1 echo "\h1 -> \h10, \h3 -> \h8 - 2 Pair Multiplexing throughput test middle links..."
h1 ./iPerfer -s -p 2011 >> ./measurements/throughput_h1-h10.txt &
h3 ./iPerfer -s -p 2012 >> ./measurements/throughput_h3-h8.txt &
h8 ./iPerfer -c -h 10.0.0.3 -p 2012 -t 20 >> ./measurements/throughput_h3-h8.txt &
h10 ./iPerfer -c -h 10.0.0.1 -p 2011 -t 20 >> ./measurements/throughput_h1-h10.txt 

h1 wait 
h3 wait 
h8 wait
h10 wait 