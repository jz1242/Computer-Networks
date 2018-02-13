if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = AssignmentNetworks()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True, controller = OVSController)

    # Run network
    net.start()
    CLI( net , script='linktest.sh' )
    net.stop()