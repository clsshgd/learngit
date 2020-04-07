How to add a service to make the app run automatically:

1. Go to the /etc/systemd/system/ directory
#cd /etc/systemd/system/


2. Add a service 

2.1 creat a service(it is recommended to take an easily recognizable name)
#vim bbu_send_data_bmc.service

2.2 the service format
A simple service needs to have three parts:[Unit],[Service] and [Install].
for example:
[Unit]
#Describe the function of the service
Description=send bbu data to BMC

[Service]
#What to do when the system starts running,the file needs to have executable permissions
ExecStart=/usr/bbuapp_send_data_BMC

[Install]
#Which mode is enabled for this service
WantedBy=multi-user.target
###Of course you can also choose other options###############


3.Start/enable service
3.1 enable it so that it will run automatically every time system boot or reboot(in multi-user mode)
#systemctl enable bbu_send_data_bmc.service

3.2 start the service
you can also enable it now
#systemctl start bbu_send_data_bmc.service

3.3 you can check its status 
#systemctl status bbu_send_data_bmc.service

3.4 stop the service
#systemctl stop bbu_send_data_bmc.service

3.5 disable the service,and it will not run automatically every time system boot or reboot
#grub mode e enter edit interface and add single in paramter F10 to run into single mode 
#or in system 
#systemctl disable bbu_send_data_bmc.service

#find use power off the bbu discharge won't be triggered
#therefore in service
#ExecStop=/home/taddeo/fortest_tmp/bbutest_disarm_bbu
#ExecStopPost=/home/taddeo/fortest_tmp/bbutest_disarm_bbu
#maybe not necessary install disarm app