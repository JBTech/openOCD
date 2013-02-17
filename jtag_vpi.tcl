source [find ./tcl/interface/jtag_vpi.cfg]
source [find ./tcl/target/or1k.cfg]

# Select the TAP core we are using
tap_select mohor

# Select the debug unit core we are using
du_select adv

gdb_port 50002

if { [catch { init } ] } {
	puts "Couldn't connect to JTAG server..."
	shutdown
} else {

	echo "Halting processor"
	halt

	foreach name [target names] {
	    set y [$name cget -endian]
	    set z [$name cget -type]
	    puts [format "Chip is %s, Endian: %s, type: %s" \
			  $name $y $z]
	}

}