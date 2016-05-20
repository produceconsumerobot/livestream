Install script for Livestream Rasberry Pi

# Get the sound lib
sudo apt-get -y install libmpg123-dev
sudo apt-get -y install alsa-utils mpg123

# Update Raspbian
sudo apt-get clean
sudo apt-get update
sudo apt-get upgrade

mkdir ~/scripts

# Get openframworks and install 
cd ~
wget http://openframeworks.cc/versions/v0.9.3/of_v0.9.3_linuxarmv6l_release.tar.gz
mkdir openFrameworks
tar vxfz of_v0.9.3_linuxarmv6l_release.tar.gz -C openFrameworks --strip-components 1
cd ~/openFrameworks/scripts/linux/debian
sudo ./install_dependencies.sh
make Release -C ~/openFrameworks/libs/openFrameworksCompiled/project
#printf 'export MAKEFLAGS=-j2 PLATFORM_VARIANT=rpi2\n' | sudo tee --append ~/.profile
cp ~/openFrameworks/examples/3d/3DPrimitivesExample/ ~/openFrameworks/apps/myApps/ -r


# Get the livestream repo
cd ~/openFrameworks/apps/
git clone https://github.com/produceconsumerobot/livestream.git

# Install I2C library and projects
sudo apt-get -y install libi2c-dev
mkdir ~/src
cd ~/src
git clone git://git.drogon.net/wiringPi
cd wiringPi
./build

# Install cmake
sudo apt-get -y install cmake

# Add I2C lines to modules file
printf 'i2c_bcm2708\n' | sudo tee --append /etc/modules
# sudo adduser pi i2c

# Get OF addons
cd ~/openFrameworks/addons/
git clone https://github.com/produceconsumerobot/ofxGPIO.git
git clone https://github.com/produceconsumerobot/ofxLidarLite.git
#git clone https://github.com/produceconsumerobot/ofxDS18B20.git


# Add lines for 1-wire protocol to modules file
#printf 'dtoverlay=w1-gpio\n' | sudo tee --append /boot/config.txt
#printf 'w1-gpio\nw1-therm' | sudo tee --append /etc/modules

# Make log directory
sudo mkdir -p /livestream/logs/
sudo chmod a+w /livestream/logs/
sudo mkdir -p /livestream/audio/
sudo chmod a+w /livestream/audio/

# automount /livestream/audio/

sudo mkdir -p /livestream


# install logmein hamachi
sudo wget https://secure.logmein.com/labs/logmein-hamachi_2.1.0.139-1_armhf.deb
sudo apt-get update
sudo apt-get upgrade
sudo apt-get -y install lsb-core
sudo dpkg -i logmein-hamachi_2.1.0.139-1_armhf.deb

# install remote desktop
sudo apt-get -y install xrdp

# install Colormake
cd ~
mkdir src
cd ~/src
git clone https://github.com/pagekite/Colormake.git
cd Colormake
sudo cp -fa colormake.pl colormake colormake-short clmake clmake-short /usr/bin/

# Configure the USB audio card
# https://learn.adafruit.com/usb-audio-cards-with-a-raspberry-pi?view=all#cm-headphone-type
printf 'pcm.!default  {\n type hw card 1\n}\nctl.!default {\n type hw card 1\n}\n' | sudo tee --append /etc/asound.conf
speaker-test -c2


