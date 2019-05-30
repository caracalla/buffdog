# Buffdog

## Setup

1. Install Debian **jessie** on a virtualbox VM
    * Tried and failed on stretch, probably due to user error
2. `apt-get install -y v86d`
3. `modprobe uvesafb`
    * The display on the VM should change to blockier text.  If this fails, check the syslog.
        * Checking the syslog: `tail /var/log/syslog`
4. Make sure you have `/dev/fb0`
    * `ls /dev | grep fb`
5. `make`

## Troubleshooting

#### "cannot reserve video memory at <address>"

Edit `/etc/modprobe.d/fbdev-blacklist.conf` and comment out any lines with "86" in them.

## Targets
* `spinner` - A simple spinning line, usually used to test that everything works.
* `buffdog` - Whatever I'm working on.  Currently, a ray tracer.

## Namesake

https://www.youtube.com/user/buffcorrell
