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

## Namesake

https://www.youtube.com/user/buffcorrell
