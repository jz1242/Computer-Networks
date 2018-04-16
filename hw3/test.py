#!/usr/bin/env python

from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel, lg, info
from mininet.node import OVSController, Node
from mininet.util import waitListening

class testNetwork(Topo):
    def __init__(self, **opts):
        """
        Create network from assignment 3 picture
        h1 miProxy client -> s1 switch -> h2 nameserver, s2 switch -> h3, h4 server
        """
        Topo.__init__(self, **opts)
        h1 = self.addHost('h1') # 10.0.0.1
        h2 = self.addHost('h2') # 10.0.0.2
        h3 = self.addHost('h3') # 10.0.0.3
        h4 = self.addHost('h4') # 10.0.0.4
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        self.addLink(h1, s1, bw=5)
        self.addLink(h2, s1, bw=5)
        self.addLink(s1, s2, bw=5)
        self.addLink(s2, h3, bw=5)
        self.addLink(s2, h4, bw=5)

def connectToRootNS( network, switch, ip, routes ):
    """Connect hosts to root namespace via switch. Starts network.
      network: Mininet() network object
      switch: switch to connect to root namespace
      ip: IP address for root namespace node
      routes: host networks to route to"""
    # Create a node in root namespace and link to switch 0
    root = Node( 'root', inNamespace=False )
    intf = network.addLink( root, switch ).intf1
    root.setIP( ip, intf=intf )
    # Start network that now includes link to root namespace
    network.start()
    # Add routes from root ns to hosts
    for route in routes:
        root.cmd( 'route add -net ' + route + ' dev ' + str( intf ) )

def sshd( network, cmd='/usr/sbin/sshd', opts='-D',
          ip='10.123.123.1/32', routes=None, switch=None ):
    """Start a network, connect it to root ns, and run sshd on all hosts.
       ip: root-eth0 IP address in root namespace (10.123.123.1/32)
       routes: Mininet host networks to route to (10.0/24)
       switch: Mininet switch to connect to root namespace (s1)"""
    if not switch:
        switch = network[ 's1' ]  # switch to use
    if not routes:
        routes = [ '10.0.0.0/24' ]
    connectToRootNS( network, switch, ip, routes )
    for host in network.hosts:
        host.cmd( cmd + ' ' + opts + '&' )
    info( "*** Waiting for ssh daemons to start\n" )
    for server in network.hosts:
        waitListening( server=server, port=22, timeout=10 )

    info( "\n*** Hosts are running sshd at the following addresses:\n" )
    for host in network.hosts:
        info( host.name, host.IP(), '\n' )
    info( "\n*** Type 'exit' or control-D to shut down network\n" )
    h3 = net.get('h3')
    h4 = net.get('h4')
    h3.cmd('python start_server.py 3')
    info("\n*** Started video server at 10.0.0.3 \n")
    h4.cmd('python start_server.py 4')
    info("\n*** Started video server at 10.0.0.4 \n")
    CLI( network )
    for host in network.hosts:
        host.cmd( 'kill %' + cmd )
    network.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = testNetwork()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
                  autoStaticArp=True, controller = OVSController)

    # Run network
    sshd( net )