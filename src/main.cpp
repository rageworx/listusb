#include <unistd.h>
#include <getopt.h>
#include <libusb.h>
#include <version.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cerrno>

#define ME_STR          "listusb"
#define VERSION_STR     "0.1.0.4"

static struct option long_opts[] = {
    { "help",           no_argument,        0, 'h' },
    { "simple",         no_argument,        0, 's' },
    { "version",        no_argument,        0, 'v' },
    { "color",          no_argument,        0, 'c' },
    { NULL, 0, 0, 0 }
};

static uint32_t        optpar_simple    = 0;
static uint32_t        optpar_color     = 0;
static libusb_context* libusbctx        = NULL;

void prtUSBclass( uint8_t id, uint8_t subid )
{
    if ( optpar_simple == 0 )
        printf( "Class = " );
    else
        printf( "cls=" );

    switch( id )
    {
        case LIBUSB_CLASS_PER_INTERFACE:
            if ( ( subid > 0 ) && ( optpar_simple == 0 ) )
            {
                printf( "PER interface %02X device.\n", subid );
            }
            else
            {
                printf( "PER/%02X;", subid ); 
            }
            break;

        case LIBUSB_CLASS_AUDIO:
            if ( optpar_simple == 0 )
                printf( "audio device" );
            else
                printf( "audio;" );
            break;

        case LIBUSB_CLASS_COMM:
            if ( optpar_simple == 0 )
                printf( "communicating device" );
            else
                printf( "communicating;" );
            break;

        case LIBUSB_CLASS_HID:
            if ( optpar_simple == 0 )
                printf( "Human Interface Device" );
            else
                printf( "HID;" );
            break;

        case LIBUSB_CLASS_PHYSICAL:
            if ( optpar_simple == 0 )
                printf( "Physical device" );
            else
                printf( "physical;" );
            break;

        case LIBUSB_CLASS_IMAGE:
            if ( optpar_simple == 0 )
                printf( "Imaging device" );
            else
                printf( "image;" );
            break;

        case LIBUSB_CLASS_PRINTER:
            if ( optpar_simple == 0 )
                printf( "Printing device" );
            else
                printf( "printer;" );
            break;

        case LIBUSB_CLASS_MASS_STORAGE:
            if ( optpar_simple == 0 )
                printf( "Mass storage device" );
            else
                printf( "mass_storage;" );
            break;

        case LIBUSB_CLASS_HUB:
            if ( optpar_simple == 0 )
                printf( "HUB device" );
            else
                printf( "HUB;" );
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
            if ( optpar_simple == 0 )
                printf( "Miscellaneous device" );
            else
                printf( "misc.;" );
            break;

        case LIBUSB_CLASS_APPLICATION:
            printf( "Application device" );
            break;

        case LIBUSB_CLASS_VENDOR_SPEC:
            printf( "Vendor-Specific device" );
            break;

        default:
            if ( optpar_simple == 0 )
                printf( "Unknown %02X class type device", id );
            else
                printf( "%02X;", id );
            break;
    }

    if ( optpar_simple == 0 )
        printf( "\n" );
}

const char* bcd2human( uint16_t id )
{
    static char retstr[16] = {0};

    uint8_t hv = id >> 8;
    uint8_t lv = ( id & 0x00F0 ) >> 4;

    if ( optpar_simple == 0 )
        snprintf( retstr, 16, "USB %u.%u", hv, lv );
    else
        snprintf( retstr, 16, "%u.%u", hv, lv );

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
            if ( optpar_simple == 0 )
                printf( "    + config[%2u] : %s, ", idx, (const char*)cfgstr );
        }
        else
        if ( optpar_simple == 0 )
        {
            printf( "    + config[%2u], ", idx );
        }

        if ( optpar_simple == 0 )
        {
            printf( "interfaces = %u, ", cfg->bNumInterfaces );
            printf( "ID = 0x%02X, ", cfg->bConfigurationValue );
        }

        uint32_t pwrCalc = cfg->MaxPower;
        if ( bcd > 0x0300 )
        {
            pwrCalc *= 8;
        }
        else
        {
            pwrCalc *= 2;
        }

        if ( optpar_simple == 0 )
            printf( "max required power = %u mA\n", pwrCalc );
        else
            printf( "MRP=%u(mA)\n", pwrCalc );
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
                if ( optpar_simple == 0 )
                {
                    printf( "Device VID:PID [%04X:%04X] ", 
                            desc.idVendor, desc.idProduct );
                }
                else
                {
                    printf( "[%04X:%04X];", desc.idVendor, desc.idProduct );
                }
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
                    if ( optpar_simple == 0 )
                        printf( "%s, ", (const char*)dev_mn );
                    else
                        printf( "%s;" , (const char*)dev_mn );
                }
                else
                {
                    if ( optpar_simple == 0 )
                        printf( "(no manufacturer)" );
                    else
                        printf( ";" );
                }


                if ( strlen( (const char*)dev_pn ) > 0 )
                {
                    if ( optpar_simple == 0)
                        printf( "%s\n", (const char*)dev_pn );
                    else
                        printf( "%s;", (const char*)dev_pn );
                }
                else
                {
                    if ( optpar_simple == 0 )
                        printf( "(no product name)\n" );
                    else
                        printf( ";" );
                }

                if ( optpar_simple == 0 )
                    printf( "    + " );

                if ( strlen( (const char*)dev_sn ) > 0 )
                {
                    if ( optpar_simple == 0 )
                        printf( "Serial number = %s\n", (const char*)dev_sn );
                    else
                        printf( "%s;", (const char*)dev_sn );
                }
                else
                {
                    if ( optpar_simple == 0 )
                        printf( "(SN not found)\n" );
                    else
                        printf( ";" ); 
                }

                if ( ( desc.bDeviceClass > 0 ) 
                        || ( desc.bDeviceSubClass > 0 ) )
                {
                    if ( optpar_simple == 0 )
                        printf( "    + " );

                    prtUSBclass( desc.bDeviceClass, desc.bDeviceSubClass );
                }
                else
                if ( optpar_simple == 1 )
                {
                    printf( "cls=none;" );
                }

                if ( optpar_simple == 0 )
                    printf( "    + " );

                uint16_t l16bcdID = libusb_cpu_to_le16( desc.bcdUSB );
                if ( optpar_simple == 0 )
                {
                    printf( "bcdID = %04X, human readable = %s", l16bcdID,
                            bcd2human( l16bcdID ) );
                    printf( "\n" );
                }
                else
                {
                    printf( "bcdID=%04X(%s);", l16bcdID, bcd2human( l16bcdID ) );
                }

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

void showHelp()
{
    const char shortusage[] = \
"Usage: listusb [-v] [-s]\n"
"\n"
"  -v,--version        display version only and quit.\n"
"  -s,--simple         display information as short as can.\n"
"  -c,--color          display with xterm-color escape codes.\n";

    fprintf( stdout, "%s\n", shortusage );    
}

void showVersion()
{
    printf( "%s version %s, libusb version %d.%d.%d\n", 
            ME_STR, VERSION_STR, LIBUSB_MAJOR, LIBUSB_MINOR, LIBUSB_MICRO );
}

int main( int argc, char** argv )
{
    // getopt
    for(;;)
    {
        int optidx = 0;
        int opt = getopt_long( argc, argv, 
                               " :hvs",
                               long_opts, &optidx );
        if ( opt >= 0 )
        {
            switch( (char)opt )
            {
                default:
                case 'h':
                    showHelp();
                    return 0;

                case 'v':
                    showVersion();
                    return 0;

                case 's':
                    optpar_simple = 1;
                    break;

                case 'c':
                    optpar_color = 1;
                    break;
            }
        }
        else
            break;
    } /// of for( == )

    if ( optpar_simple == 0 ) \
    printf( "%s, version %s, (C)Copyrighted 2023 Raphael Kim, w/ libusb %d.%d.%d\n", 
            ME_STR, VERSION_STR, LIBUSB_MAJOR, LIBUSB_MINOR, LIBUSB_MICRO );

    libusb_init( &libusbctx );

    if ( libusbctx != NULL )
    {
        size_t devs = listdevs();

        if ( ( devs > 0 ) && ( optpar_simple == 0 ) )
            printf( "total %zu devices found.\n", devs );

        fflush( stdout );

        libusb_exit( libusbctx );
    }
    else
    {
        fprintf( stderr, "libusb context should not initialized.\n" );
    }

    return 0;
}
