# spice-conduit-d
This software is a prototype not suitable for production.

The purpose of this software is to implement the 'Conduit' concept in the VM side (see wiki https://github.com/720k/virt-viewer/wiki ). 

spice-conduit-d windows service redirect data to system port \\.\global\io.bplayer.data.0 to \\.\pipe\io.bplayer.data.0 

Context: Inside Windows virtual machine connected with Spice protocol to KVM/QEMU ( https://libvirt.org/drvqemu.html ) .


Building:
-  project built by QtCreator, QT framework https://www.qt.io/

Dependencies:
- QtService ( https://github.com/Skycoder42/QtService )

  
