Install script for Livestream Rasberry Pi

# Get the sound lib
sudo apt-get -y install libmpg123-dev


mkdir ~/scripts

# Get openframworks and install 
cd ~
curl -O http://www.openframeworks.cc/versions/v0.8.4/of_v0.8.4_linuxarmv7l_release.tar.gz
tar xvf of_v0.8.4_linuxarmv7l_release.tar.gz
curl https://raw.githubusercontent.com/openframeworks/openFrameworks/master/libs/openFrameworksCompiled/project/linuxarmv7l/config.linuxarmv7l.rpi2.mk -o of_v0.8.4_linuxarmv7l_release/libs/openFrameworksCompiled/project/linuxarmv7l/config.linuxarmv7l.rpi2.mk
cd ~/of_v0.8.4_linuxarmv7l_release/scripts/linux/debian/
cp -f ~/scripts/install_dependencies.sh .
sudo ./install_dependencies.sh
printf 'export MAKEFLAGS=-j4 PLATFORM_VARIANT=rpi2\n' | sudo tee --append ~/.profile
cp ~/of_v0.8.4_linuxarmv7l_release/examples/3d/3DPrimitivesExample/ ~/of_v0.8.4_linuxarmv7l_release/apps/myApps/ -r

# Get the livestream repo
cd ~/of_v0.8.4_linuxarmv7l_release/apps/
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
printf 'i2c_bcm2708\ni2c-dev\n' | sudo tee --append /etc/modules
# sudo adduser pi i2c

# Get OF addons
cd ~/of_v0.8.4_linuxarmv7l_release/addons/
git clone https://github.com/produceconsumerobot/ofxGPIO.git
git clone https://github.com/produceconsumerobot/ofxLidarLite.git
git clone https://github.com/produceconsumerobot/ofxDS18B20.git


# Add lines for 1-wire protocol to modules file
printf 'dtoverlay=w1-gpio\n' | sudo tee --append /boot/config.txt
printf 'w1-gpio\nw1-therm' | sudo tee --append /etc/modules

# Make log directory
sudo mkdir -p /logs/livestream/
sudo chmod a+w /logs/livestream/

# install logmein hamachi
sudo wget https://secure.logmein.com/labs/logmein-hamachi_2.1.0.139-1_armhf.deb
sudo apt-get update
sudo apt-get upgrade
sudo apt-get -y install lsb-core
sudo dpkg -i logmein-hamachi_2.1.0.139-1_armhf.deb

# install remote desktop
sudo apt-get -y install xrdp


