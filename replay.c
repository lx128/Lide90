#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pcap/pcap.h>
#include <pcap/usb.h>
#include <libusb-1.0/libusb.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_DATA_DUMP 16

typedef unsigned char uchar;

struct libusb_device_handle *dev_handle; //a device handle

int main(int argc, char *argv[])
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *p;
	struct pcap_pkthdr *pkt_header;
	pcap_usb_header_mmapped *usb_header;
	libusb_context *ctx = NULL; //a libusb session
	int r, r2, csv_file, img_file, comp;
	uchar data_in[32768];
	uchar hex[32768];
	u_char *data_out;
	const u_char *pkt_data; //data to write
	struct timeval record_elapsed_time, record_saved_time;

	r = libusb_init(&ctx); //initialize a library session

	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x4a9, 0x1900); //these are vendorID and productID
    if(dev_handle == NULL)
	{
		printf("Cannot open device\n");
		return 1;
	}
	else
        printf("Device Opened\n");

    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }

    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) {
        printf("Cannot Claim Interface\n");
        return 1;
    }
    printf("Claimed Interface\n");

	if(argc!=2)
	{return 1;}

	p = pcap_open_offline(argv[1],errbuf);
	if(p==NULL)
	{
		printf("File not found\n");
		return 1;
	}
	
	csv_file = open("output.csv", O_CREAT|O_WRONLY|O_TRUNC);
	if(csv_file==0)
	{
		printf("%s\n",strerror(csv_file));
		return 1;
	}
	img_file = open("img.data", O_CREAT|O_WRONLY|O_TRUNC);
	if(img_file==0)
	{
		printf("%s\n",strerror(img_file));
		return 1;
	}

    printf("Start playing...\n");

	r=snprintf((char *)data_in,32768,"No;Type;bmReq;bReq;wValue;windex;wlengh;data\n");
	write(csv_file, data_in, r);

	timerclear(&record_saved_time);//clear
	comp=0;
	while((r=pcap_next_ex(p, &pkt_header, &pkt_data))>=0)// && comp<100)
	{
		comp++;
		usb_header = (pcap_usb_header_mmapped*)pkt_data;

		if((usb_header->event_type=='S')//on ne s'occupe que des Submit, pas des réponses du fichier pcap
		& (usb_header->s.setup.bmRequestType!=0x80)//evite requete config et device
		//& (usb_header->s.setup.wValue!=0x8c)//ignore int, empeche le scan rapide

		& (usb_header->s.setup.wValue!=0x8e)//envoi requete
		//& (usb_header->s.setup.wValue!=0x84)//si retiré, empeche le scan complet

//pour fichier w_preview
	//	& (comp>1900)//config
	//	& (comp<2863)//a 2864 ca demarre

	//	& (comp>2864)//config 2
	//	& (comp<3136)//

	//	& (comp>3137)//lancement scan. A 2470 ca echoue
		//& (comp<3181)//block recup des donnees et avance du scan

		//& (comp>3350)//retour chariot
		//& (comp<3670)

//pour fichier w_init
		//& (comp>1900)//
		//& (comp<3000)//
		)
		{
			if(timerisset(&record_saved_time))
			{
				timersub(&pkt_header->ts,&record_saved_time,&record_elapsed_time);
			}
			else//init
			{
				timerclear(&record_elapsed_time);
			}

			//printf("%ld ",record_elapsed_time.tv_sec);
			//printf("%ld\n",record_elapsed_time.tv_usec);

			usleep((record_elapsed_time.tv_sec*1000000)+record_elapsed_time.tv_usec);

			switch (usb_header->transfer_type)
			{
				case URB_CONTROL:
					switch (usb_header->endpoint_number)
					{
						case 0x00://OUT
							data_out = ((uchar*)usb_header + sizeof(pcap_usb_header_mmapped));
							r2=libusb_control_transfer(dev_handle, usb_header->s.setup.bmRequestType, usb_header->s.setup.bRequest, usb_header->s.setup.wValue, usb_header->s.setup.wIndex, data_out, usb_header->s.setup.wLength, 0);

							//Ecriture des données brutes
							//write(csv_file, data_out, usb_header->s.setup.wLength);

							r=snprintf((char *)data_in,32768,"%d;Control OUT;0x%X;0x%X;0x%X;0x%X;0x%hX;", comp,usb_header->s.setup.bmRequestType, usb_header->s.setup.bRequest, usb_header->s.setup.wValue, usb_header->s.setup.wIndex, usb_header->s.setup.wLength);
							write(csv_file, data_in, r);

							//conversion des data en Hex et ajout dans le csv
							r=0;
							for (int i = 0; i < usb_header->s.setup.wLength && i<MAX_DATA_DUMP; i ++)
							{
								r+=snprintf((char *)data_in+r,32768-r,"0x%X ", data_out[i]);
							}
							write(csv_file, data_in, r);

							//retour ligne
							write(csv_file, "\n", 1);
						break;
						case 0x80://IN
							r2=libusb_control_transfer(dev_handle, usb_header->s.setup.bmRequestType, usb_header->s.setup.bRequest, usb_header->s.setup.wValue, usb_header->s.setup.wIndex, data_in, usb_header->s.setup.wLength, 0);

							//Ecriture des données brutes
							//write(csv_file, data_in, usb_header->s.setup.wLength);

							r=snprintf((char *)hex,32768,"%d;Control IN;0x%X;0x%X;0x%X;0x%X;0x%hX;", comp,usb_header->s.setup.bmRequestType, usb_header->s.setup.bRequest, usb_header->s.setup.wValue, usb_header->s.setup.wIndex, usb_header->s.setup.wLength);
							write(csv_file, hex, r);

							//conversion des data en Hex et ajout dans le csv
							r=0;
							for (int i = 0; i < r2 && i<MAX_DATA_DUMP; i ++)
							{
								r+=snprintf((char *)hex+r,32768-r,"0x%X ", data_in[i]);
							}
							write(csv_file, hex, r);

							//retour ligne
							write(csv_file, "\n", 1);
						break;
					}

				break;
				case URB_BULK:
					switch (usb_header->endpoint_number & 0x80)
					{
						case 0x00://OUT
							data_out = ((uchar*)usb_header + sizeof(pcap_usb_header_mmapped));
							r2=libusb_bulk_transfer(dev_handle, usb_header->endpoint_number, data_out, usb_header->urb_len, (int *)&usb_header->data_len, 0);

							//Ecriture des données brutes
							//write(csv_file, data_out, usb_header->data_len);

							r=snprintf((char *)data_in,32768,"%d;Data bulk OUT;;;;;0x%hX;", comp, usb_header->data_len);
							write(csv_file, data_in, r);

							//conversion des data en Hex et ajout dans le csv
							/*r=0;
							for (int i = 0; i < usb_header->data_len && i<MAX_DATA_DUMP; i ++)
							{
								r+=snprintf((char *)data_in+r,32768-r,"0x%X ", data_out[i]);
							}
							write(csv_file, data_in, r);*/

							//retour ligne
							write(csv_file, "\n", 1);
						break;
						case 0x80://IN
							r2=libusb_bulk_transfer(dev_handle, usb_header->endpoint_number, data_in, usb_header->urb_len, (int *)&usb_header->data_len, 0);

							//Ecriture des données brutes
							write(img_file, data_in, usb_header->data_len);

							r=snprintf((char *)data_in,32768,"%d;Data bulk IN;;;;;0x%hX;", comp, usb_header->data_len);
							write(csv_file, data_in, r);

							//conversion des data en Hex et ajout dans le csv
							/*r=0;
							for (int i = 0; i < usb_header->data_len && i<MAX_DATA_DUMP; i ++)
							{
								r+=snprintf((char *)hex+r,32768-r,"0x%X ", data_in[i]);
							}
							write(csv_file, hex, r);*/

							//retour ligne
							write(csv_file, "\n", 1);
						break;
					}
				break;
			}
			
		}
		if (r2 < 0)//erreur lors de la requete USB
		{
			printf("Error USB on request: %d %s\n", r2, libusb_error_name(r2));
			break;
		}
		record_saved_time=pkt_header->ts;
	}

    if (r == -1)
    {
        printf("Error reading the packets: %s\n", pcap_geterr(p));
    }

    r = libusb_release_interface(dev_handle, 1); //release the claimed interface
    if(r<0) {
        printf("Cannot Release Interface\n");
		printf("Error USB: %d %s\n", r, libusb_error_name(r));
    }
    else{
		printf("Released Interface\n");
	}

    libusb_close(dev_handle); //close the device we opened
	printf("Comp %d\n",comp);

	libusb_exit(ctx); //close the session
	close(csv_file);
	return 0;
}
