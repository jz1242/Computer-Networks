running exercise 2
-put source_routing in p4_src
-put commands.txt  in the directory outside
-put kv.py and recieve.py in the directory outside as well
-run run_demo
-inside mininet run xterm h1 h1
-on h1 run python kv.py
-on h3 run python receive.py
-you should now see messages go through