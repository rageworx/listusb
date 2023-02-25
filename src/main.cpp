#include <unistd.h>
#include <libusb.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

static libusb_context* libusbctx = NULL;

void prtUSBclass( uint8_t id, uint8_t subid )
{
    printf( "CLASS=" );

    switch( id )
    {
        case LIBUSB_CLASS_PER_INTERFACE:
            if ( subid > 0 )
            {
                printf( "PER interface %02X device.\n", subid );
            }
            break;

        case LIBUSB_CLASS_AUDIO:
            printf( "audio device" );
            break;

        case LIBUSB_CLASS_COMM:
            printf( "communicating device" );
            break;

        case LIBUSB_CLASS_HID:
            printf( "Human Interface Device" );
            break;

        case LIBUSB_CLASS_PHYSICAL:
            printf( "Physical device" );
            break;

        case LIBUSB_CLASS_IMAGE:
            printf( "Imaging device" );
            break;

        case LIBUSB_CLASS_PRINTER:
            printf( "Printing device" );
            break;

        case LIBUSB_CLASS_MASS_STORAGE:
            printf( "Mass storage device" );
            break;

        case LIBUSB_CLASS_HUB:
            printf( "HUB device" );
            break;

        case LIBUSB_CLASS_DATA:
            printf( "Data device" );
            break;

        case LIBUSB_CLASS_SMART_CARD:
            printf( "Smart Card device" );
            break;

        case LIBUSB_CLASS_CONTENT_SECURITY:
            printf( "Content Security device" );
            break;

        case LIBUSB_CLASS_VIDEO:
            printf( "Video device" );
            break;

        case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
            printf( "Personal Healthcare device" );
            break;

        case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
            printf( "Diagnositc device" );
            break;

        case LIBUSB_CLASS_WIRELESS:
            printf( "Wireless device" );
            break;

        case LIBUSB_CLASS_MISCELLANEOUS:
            printf( "Miscellaneous device" );
            break;

        case LIBUSB_CLASS_APPLICATION:
            printf( "Application device" );
            break;

        case LIBUSB_CLASS_VENDOR_SPEC:
            printf( "Vendor-Specific device" );
            break;

        default:
            printf( "Unknown %02X class type device", id );
            break;
    }

    printf( "\n" );
}

const char* bcd2human( uint16_t id )
{
    static char retstr[16] = {0};

    uint8_t hv = id >> 8;
    uint8_t lv = ( id & 0x00F0 ) >> 4;

    snprintf( retstr, 16, "USB %u.%u", hv, lv );

    return retstr;
}

void prtUSBConfig( libusb_device_handle* dev, uint8_t idx, uint16_t bcd, libusb_config_descriptor* cfg )
{
    if ( cfg != NULL )
    {
        uint8_t cfgstr[64] = {0};

        if ( cfg->bDescriptorType == LIBUSB_DT_STRING )
        {
            libusb_get_string_descriptor_ascii( dev, 
                                                cfg->iConfiguration,
                                                cfgstr, 64 );
        }

        if ( strlen( (const char*)cfgstr ) > 0 )
        {    
            printf( "    + config[%2u] : %s, ", idx, (const char*)cfgstr );
        }
        else
        {
            printf( "    + config[%2u] , ", idx );
        }
        printf( "interfaces=%u, ", cfg->bNumInterfaces );
        printf( "ID=0x%02X, ", cfg->bConfigurationValue );
        uint32_t pwrCalc = cfg->MaxPower;
        if ( bcd > 0x0300 )
        {
            pwrCalc *= 8;
        }
        else
        {
            pwrCalc *= 2;
        }
        printf( "max req. power=%u mA\n", pwrCalc );
    }
}

size_t listdevs()
{
    libusb_device_handle* dev = NULL;
    libusb_device** listdev = NULL;
    size_t devscnt = libusb_get_device_list( libusbctx, &listdev );

    if ( devscnt > 0 )
    {
        for ( size_t cnt = 0; cnt<devscnt; cnt++ )
        {
            libusb_device* device = listdev[cnt];
            libusb_device_descriptor desc = {0};
            libusb_config_descriptor* cfg;

            uint8_t dev_pn[128] = {0};
            uint8_t dev_mn[128] = {0};
            uint8_t dev_sn[64] = {0};

            if ( libusb_get_device_descriptor( device, &desc ) == 0 )
            {
                printf( "Device VID:PID [%04X:%04X] ", 
                        desc.idVendor, desc.idProduct );
                // open device ..
                int usberr = libusb_open( device, &dev );
                if ( usberr == 0 )
                {
                    libusb_get_string_descriptor_ascii( dev,
                                                        desc.iProduct,
                                                        dev_pn,
                                                        128 );

                    libusb_get_string_descriptor_ascii( dev,
                                                  desc.iManufacturer,
                                                  dev_mn,
                                                  128 );

                    libusb_get_string_descriptor_ascii( dev,
                                                  desc.iSerialNumber,
                                                  dev_sn,
                                                  64 );

                }

                if ( strlen( (const char*)dev_mn ) > 0 )
                {
                    printf( "%s, ", (const char*)dev_mn );
                }
                else
                {
                    printf( "(no manufacturer)" );
                }


                if ( strlen( (const char*)dev_pn ) > 0 )
                {
                    printf( "%s\n", (const char*)dev_pn );
                }
                else
                {
                    printf( "(no product name)\n" );
                }

                printf( "    + " );

                if ( strlen( (const char*)dev_sn ) > 0 )
                {
                    printf( "Serial number = %s\n", (const char*)dev_sn );
                }
                else
                {
                    printf( "(SN not found)\n" );
                }

                if ( ( desc.bDeviceClass > 0 ) 
                        || ( desc.bDeviceSubClass > 0 ) )
                {
                    printf( "    + " );
                    prtUSBclass( desc.bDeviceClass, desc.bDeviceSubClass );
                }

                printf( "    + " );
                uint16_t l16bcdID = libusb_cpu_to_le16( desc.bcdUSB );
                printf( "bcdID=%04X, human readable = %s", l16bcdID,
                        bcd2human( l16bcdID ) );

                printf( "\n" );

                // get config
                if ( desc.bNumConfigurations > 0 )
                {
                    for ( uint8_t cnt=0; cnt<desc.bNumConfigurations; cnt++ )
                    {
                        usberr = libusb_get_config_descriptor( device, 
                                                               cnt,
                                                               &cfg );
                        if ( usberr == 0 )
                        {
                            prtUSBConfig( dev, cnt, l16bcdID, cfg );
                        }
                    }
                }

                if ( dev != NULL )
                    libusb_close( dev );
            }
        }
    }

    return devscnt;
}

int main( int argc, char** argv )
{
    libusb_init( &libusbctx );

    size_t devs = listdevs();

    fflush( stdout );

    libusb_exit( libusbctx );

    return 0;
}
