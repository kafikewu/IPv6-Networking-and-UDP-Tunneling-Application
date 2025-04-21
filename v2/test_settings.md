# pingc
    ssh khan555@amber01.cs.purdue.edu
    cd /homes/khan555/cs536/lab4/v2
    pingc 128.10.112.133 60005 128.10.112.131
# tunnelc
    ssh khan555@amber02.cs.purdue.edu
    cd /homes/khan555/cs536/lab4/v2
    tunnelc 128.10.112.133 55550 abcdABCD 128.10.112.131 128.10.112.134 56001
# tunnels
    ssh khan555@amber03.cs.purdue.edu
    cd /homes/khan555/cs536/lab4/v2
    tunnels 128.10.112.133 55550 abcdABCD
    ## Helpful Commands
        lsof -i :<port>
        e.g. lsof -i :55550
        kill -9 <PID>
# pings
    ssh khan555@amber04.cs.purdue.edu
    cd /homes/khan555/cs536/lab4/v2
    pings 128.10.112.134 56001

