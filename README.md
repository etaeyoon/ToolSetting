# ToolSetting

sudo apt-get install vim
Plug: https://github.com/junegunn/vim-plug

ref. vimrc file


sudo apt-get install git gitk

cuda install
1. Remove nouveau 
ref. https://askubuntu.com/questions/841876/how-to-disable-nouveau-kernel-driver

$ sudo apt-get remove nvidia* && sudo apt autoremove
$ sudo apt-get install dkms build-essential linux-headers-generic

Added items in blacklist.conf
$ sudo vi /etc/modprobe.d/blacklist.conf
blacklist nouveau
blacklist lbm-nouveau
options nouveau modeset=0
alias nouveau off
alias lbm-nouveau off

$ echo options nouveau modeset=0 | sudo tee -a /etc/modprobe.d/nouveau-kms.conf

$ sudo update-initramfs -u
$ sudo reboot

2. Install cuda
Download and install cuda.

Added items in ~/.bashrc
export PATH=$PATH:/usr/local/cuda-version
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda-version/lib64

$ cd /usr/bin/
$ sudo ln -s /usr/local/cuda-version/bin/nvcc

$ sudo reboot
