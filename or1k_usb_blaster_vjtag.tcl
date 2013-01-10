# Choose Altera USB-Blaster as JTAG interface
source [find ./tcl/interface/altera-usb-blaster.cfg]
usb_blaster pin6 1
usb_blaster pin8 1

# Configure the target
source [find ./tcl/target/vjtag.cfg]

# Set GDB port
gdb_port 50001

# Select the TAP core we are using
tap_select vjtag

# Select the debug unit core we are using
du_select adv

# Add a new register in the cpu register list. This register will be
# included in the generated target descriptor file.
# format is addreg [name] [address] [feature] [reg_group]
addreg rtest 0x1234 group0 system

# Target initialization
init
echo "Halting processor"
halt

foreach name [target names] {
    set y [$name cget -endian]
    set z [$name cget -type]
    puts [format "Chip is %s, Endian: %s, type: %s" \
                  $name $y $z]
}

set c_blue  "\033\[01;34m"
set c_reset "\033\[0m"

puts [format "%sTarget ready...%s" $c_blue $c_reset]
