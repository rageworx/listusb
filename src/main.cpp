#include <unistd.h>
#include <getopt.h>
#include <libusb.h>
#include <version.h>
#include <version_nano.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cerrno>

#define ME_STR          "listusb"
#define VERSION_STR     "0.2.0.10"

static struct option long_opts[] = {
    { "help",           no_argument,        0, 'h' },
    { "simple",         no_argument,        0, 's' },
    { "reftable",       no_argument,        0, 't' },
    { "version",        no_argument,        0, 'v' },
    { "color",          no_argument,        0, 'c' },
    { NULL, 0, 0, 0 }
};

static uint32_t        optpar_reftbl    = 0;
static uint32_t        optpar_simple    = 0;
static uint32_t        optpar_color     = 0;
static libusb_context* libusbctx        = NULL;

void prtUSBclass( uint8_t id, uint8_t subid )
{
    if ( optpar_color > 0 )
    {
        printf( "\033[94m" );
    }

    if ( optpar_simple == 0 )
    {
        printf( "Class = " );
    }
    else
    {
        printf( "cls=" );
    }

    if ( optpar_color > 0 )
    {
        printf( "\033[93m" );
    }

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
            if ( optpar_simple == 0 )
                printf( "Data device" );
            else
                printf( "data;" );
            break;

        case LIBUSB_CLASS_SMART_CARD:
            if ( optpar_simple == 0 )
                printf( "Smart Card device" );
            else
                printf( "smartcard;" );
            break;

        case LIBUSB_CLASS_CONTENT_SECURITY:
            if ( optpar_simple == 0 )
                printf( "Content Security device" );
            else
                printf( "content_security;" );
            break;

        case LIBUSB_CLASS_VIDEO:
            if ( optpar_simple == 0 )
                printf( "Video device" );
            else
                printf( "video;" );
            break;

        case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
            if ( optpar_simple == 0 )
                printf( "Personal Healthcare device" );
            else
                printf( "personal_healthcare;" );
            break;

        case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
            if ( optpar_simple == 0 )
                printf( "Diagnositc device" );
            else
                printf( "diagnostic;" );
            break;

        case LIBUSB_CLASS_WIRELESS:
            if ( optpar_simple == 0 )
                printf( "Wireless device" );
            else
                printf( "wireless;" );
            break;

        case LIBUSB_CLASS_MISCELLANEOUS:
            if ( optpar_simple == 0 )
                printf( "Miscellaneous device" );
            else
                printf( "misc.;" );
            break;

        case LIBUSB_CLASS_APPLICATION:
            if ( optpar_simple == 0 )
                printf( "Application device" );
            else
                printf( "application;" );
            break;

        case LIBUSB_CLASS_VENDOR_SPEC:
            if ( optpar_simple == 0 )
                printf( "Vendor-Specific device" );
            else
                printf( "vendor-spec;" );
            break;

        default:
            if ( optpar_simple == 0 )
                printf( "Unknown %02X class type device", id );
            else
                printf( "%02X;", id );
            break;
    }

    if ( optpar_color > 0 )
    {
        printf( "\033[0m" );
    }

    if ( optpar_simple == 0 )
        printf( "\n" );
}

const char* bcd2human( uint16_t id )
{
    static char retstr[32] = {0};

    uint8_t hv = id >> 8;
    uint8_t lv = ( id & 0x00F0 ) >> 4;

    if ( optpar_simple == 0 )
    {
        if ( optpar_color > 0 )
        {
            snprintf( retstr, 32, "\033[93mUSB \033[91m%u.%u", hv, lv );
        }
        else
        {
            snprintf( retstr, 32, "USB %u.%u", hv, lv );
        }
    }
    else
    {
        if ( optpar_color > 0 )
        {
            snprintf( retstr, 32, "\033[91m%u.%u", hv, lv );
        }
        else
        {
            snprintf( retstr, 32, "%u.%u", hv, lv );
        }
    }

    return retstr;
}

void prtUSBConfig( libusb_device* device, libusb_device_handle* dev, uint8_t idx, uint16_t bcd, libusb_config_descriptor* cfg )
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

        if ( optpar_color > 0 )
        {
            printf( "\033[93m" );
        }

        if ( optpar_simple == 0 )
            printf( "    + ");

        if ( strlen( (const char*)cfgstr ) > 0 )
        {
            if ( optpar_simple == 0 )
            {
                if ( optpar_color > 0 )
                {
                    printf( "\033[94m" );
                }

                printf( "config[" );

                if ( optpar_color > 0 )
                {
                    printf( "\033[95m" );
                }

                printf( "%2u", idx );

                if ( optpar_color > 0 )
                {
                    printf( "\033[94m" );
                }

                printf( "] " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[0m" );
                }

                printf( " : " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }

                printf( "%s, ", (const char*)cfgstr );
            }
        }
        else
        if ( optpar_simple == 0 )
        {
            if ( optpar_color > 0 )
            {
                printf( "\033[94m" );
            }

            printf( "config[" );

            if ( optpar_color > 0 )
            {
                printf( "\033[95m" );
            }

            printf( "%2u", idx );

            if ( optpar_color > 0 )
            {
                printf( "\033[94m" );
            }

            printf( "], ");
        }

        if ( optpar_simple == 0 )
        {
            if ( optpar_color > 0 )
            {
                printf( "\033[94m" );
            }

            printf( "interfaces = " );

            if ( optpar_color > 0 )
            {
                printf( "\033[93m" );
            }

            printf( "%u, ", cfg->bNumInterfaces );

            if ( optpar_color > 0 )
            {
                printf( "\033[95m" );
            }

            printf( "ID = " );

            if ( optpar_color > 0 )
            {
                printf( "\033[93m" );
            }

            printf( "0x%02X, ", cfg->bConfigurationValue );
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

        if ( optpar_color > 0 )
        {
            printf( "\033[94m" );
        }

        if ( optpar_simple == 0 )
            printf( "max required power = " );
        else
            printf( "MRP=" );

        if ( optpar_color > 0 )
        {
            printf( "\033[93m" );
        }

        if ( optpar_simple == 0 )
            printf( "%u mA\n", pwrCalc );
        else
            printf( "%u(mA)\n", pwrCalc );

        if ( optpar_color > 0 )
        {
            printf( "\033[0m" );
        }

        // testing interfaces ...
        if ( ( cfg->bNumInterfaces > 0 ) && ( optpar_simple == 0 ) )
        {
            for ( int x=0; x<cfg->bNumInterfaces; x++ )
            {
                if ( optpar_color > 0 )
                {
                    printf( "\033[97m" );
                }

                printf( "        - interface[" );

                if ( optpar_color > 0 )
                {
                    printf( "\033[96m" );
                }

                printf( "%d",x );

                if ( optpar_color > 0 )
                {
                    printf( "\033[97m" );
                }

                printf( "] : " );

                if ( cfg->extra_length > 0 )
                {
                    printf( "%s, ", (const char*)cfg->extra );
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[96m" );
                }

                printf( "alt.settings = " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }

                printf( "%d",cfg->interface[x].num_altsetting );

                if ( optpar_color > 0 )
                {
                    printf( "\033[97m" );
                }

                printf( " -> " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[96m" );
                }

                printf( "(" );

                for( int y=0; y<cfg->interface[x].num_altsetting; y++ )
                {
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[96m" );
                    }

                    printf( "ep[" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[95m" );
                    }

                    printf( "%d", y );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[96m" );
                    }

                    printf( "]" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[91m" );
                    }

                    printf( "=" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[92m" );
                    }

                    printf( "%d", cfg->interface[x].altsetting[y].bNumEndpoints );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[95m" );
                    }

                    printf( ":" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[32m" );
                    }

                    for ( int z=0; z<cfg->interface[x].altsetting[y].bNumEndpoints; z++ )
                    {
                        if ( cfg->interface[x].altsetting[y].endpoint[z].extra_length > 0 )
                        {
                            const char* pE = (const char*)cfg->interface[x].altsetting[y].endpoint[z].extra;

                            if ( ( *pE >= '0' ) && ( *pE <= '9' ) )
                            {
                                printf( "%c", *pE );
                            }
                            else
                            {
                                printf( "0x%X", (uint8_t)*pE );
                            }
                        }
                        else
                        {
                            printf( "-" );
                        }

                        if (z+1 < cfg->interface[x].altsetting[y].bNumEndpoints )
                        {
                            printf( "," );
                        }
                    }

                    if( y+1 < cfg->interface[x].num_altsetting )
                    {
                        if ( optpar_color > 0 )
                        {
                            printf( "\033[0m" );
                        }
                        printf( "," );
                    }
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[96m" );
                }

                printf( ")\n" );

                if ( optpar_color > 0 )
                {
                    printf( "\033[0m" );
                }
            }
        } /// of if ( ( cfg->bNumInterfaces > 0 ) && ( optpar_simple == 0 ) )
    }
}

size_t listdevs()
{
    libusb_device_handle* dev = NULL;
    libusb_device** listdev = NULL;
    size_t devscnt = libusb_get_device_list( libusbctx, &listdev );

    if ( devscnt > 0 )
    {
        if ( ( optpar_simple > 0 ) && ( optpar_reftbl > 0 ) )
        {
            if ( optpar_color > 0 )
            {
                printf( "\033[93m" );
            }
            printf( "BUS;" );

            if ( optpar_color > 0 )
            {
                printf( "\033[97m" );
            }
            printf( "Port;");

            if ( optpar_color > 0 )
            {
                printf( "\033[92m" );
            }
            printf( "[ PID: VID]; ");

            if ( optpar_color > 0 )
            {
                printf( "\033[91m" );
            }
            printf( "manufacturer; ");

            if ( optpar_color > 0 )
            {
                printf( "\033[95m" );
            }
            printf( "product name; " );

            if ( optpar_color > 0 )
            {
                printf( "\033[91m" );
            }
            printf( "serial No.; " );

            if ( optpar_color > 0 )
            {
                printf( "\033[94m" );
            }
            printf( "class; " );

            if ( optpar_color > 0 )
            {
                printf( "\033[93m" );
            }
            printf( "bcdID; " );

            if ( optpar_color > 0 )
            {
                printf( "\033[97m" );
            }
            printf( "MRP(mA)\n" );

            if ( optpar_color > 0 )
            {
                printf( "\033[0m" );
            }
        }

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
                uint8_t dev_bus = libusb_get_bus_number( device );
                uint8_t dev_port = libusb_get_port_number( device );

                if ( optpar_simple == 0 )
                {
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[94m" );
                    }
                    printf( "Bus " );
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }
                    printf( "%03u, ", dev_bus );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }
                    printf( "Port " );
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[97m" );
                    }
                    printf( "%03u ", dev_port );
                }
                else
                {
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }
                    printf( "%03u;", dev_bus );
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[97m" );
                    }
                    printf( "%03u;", dev_port );
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[92m" );
                }

                if ( optpar_simple == 0 )
                {
                    printf( "[%04X:%04X] ", desc.idVendor, desc.idProduct );
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
                else
                {
                    dev = NULL;
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[91m" );
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

                if ( optpar_color > 0 )
                {
                    printf( "\033[95m" );
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

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }

                if ( optpar_simple == 0 )
                    printf( "    + " );

                if ( strlen( (const char*)dev_sn ) > 0 )
                {
                    if ( optpar_simple == 0 )
                    {
                        if ( optpar_color > 0 )
                        {
                            printf( "\033[94m" );
                        }

                        printf( "Serial number =" );

                        if ( optpar_color > 0 )
                        {
                            printf( "\033[93m" );
                        }

                        printf(" %s\n", (const char*)dev_sn );
                    }
                    else
                        printf( "%s;", (const char*)dev_sn );
                }
                else
                {
                    if ( optpar_simple == 0 )
                    {
                        if ( optpar_color > 0 )
                        {
                            printf( "\033[91m" );
                        }

                        printf( "(SN not found)\n" );
                    }
                    else
                        printf( ";" );
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
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
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[94m" );
                    }

                    printf( "cls=" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }

                    printf( "none;" );
                }

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }

                if ( optpar_simple == 0 )
                    printf( "    + " );

                uint16_t l16bcdID = libusb_cpu_to_le16( desc.bcdUSB );
                if ( optpar_simple == 0 )
                {
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[94m" );
                    }

                    printf( "bcdID = " );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }

                    printf( "%04X,", l16bcdID );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[94m" );
                    }

                    printf( " human readable = " );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }

                    printf( "%s",
                            bcd2human( l16bcdID ) );
                    printf( "\n" );
                }
                else
                {
                    if ( optpar_color > 0 )
                    {
                        printf( "\033[94m" );
                    }

                    printf( "bcdID=" );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }

                    printf( "%04X(", l16bcdID );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[97m" );
                    }

                    printf( "%s", bcd2human( l16bcdID ) );

                    if ( optpar_color > 0 )
                    {
                        printf( "\033[93m" );
                    }

                    printf( ");" );
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
                            prtUSBConfig( device, dev, cnt, l16bcdID, cfg );
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
"  -c,--color          display with xterm-color escape codes.\n"
"  -t,--reftable       display reference table section with --simple.\n";

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
                               " :hvsct",
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

                case 't':
                    optpar_reftbl = 1;
                    break;
            }
        }
        else
            break;
    } /// of for( == )

    if ( optpar_simple == 0 )
    {
        if ( optpar_color > 0 )
        {
            printf( "\033[93m" );
        }
        printf( "%s", ME_STR );
        if ( optpar_color > 0 )
        {
            printf( "\033[0m" );
        }
        printf( ", " );

        if ( optpar_color > 0 )
        {
            printf( "\033[96m" );
        }
        printf( "version %s", VERSION_STR );
        if ( optpar_color > 0 )
        {
            printf( "\033[0m" );
        }
        printf( ", " );

        if ( optpar_color > 0 )
        {
            printf( "\033[94m" );
        }
        printf( "(C)Copyrighted 2023 Raphael Kim" );
        if ( optpar_color > 0 )
        {
            printf( "\033[0m" );
        }
        printf( ", " );

        if ( optpar_color > 0 )
        {
            printf( "\033[93m" );
        }
        printf( "w/ libusb v%d.%d.%d", LIBUSB_MAJOR, LIBUSB_MINOR, LIBUSB_MICRO );
        if ( optpar_color > 0 )
        {
            printf( "\033[0m" );
        }
        printf( "\n" );
    }

#if (LIBUSB_NANO>11780)
    libusb_init_option lusbopt[1];
    lusbopt[0].option = LIBUSB_OPTION_LOG_LEVEL;
    lusbopt[0].value.ival = 0;
    libusb_init_context( &libusbctx, lusbopt, 1 );
#else
    libusb_init_context( &libusbctx );
#endif

    if ( libusbctx != NULL )
    {
        size_t devs = listdevs();

        if ( ( devs > 0 ) && ( optpar_simple == 0 ) )
        {
            if ( optpar_color > 0 )
            {
                printf( "\033[96m" );
            }

            printf( "total " );

            if ( optpar_color > 0 )
            {
                printf( "\033[93m" );
            }

            printf( "%zu", devs );

            if ( optpar_color > 0 )
            {
                printf( "\033[96m" );
            }

            if ( devs == 1 )
                printf( " device found.\n" );
            else
                printf( " devices found.\n" );

            if ( optpar_color > 0 )
            {
                printf( "\033[0m" );
            }
        }

        fflush( stdout );

        libusb_exit( libusbctx );
    }
    else
    {
        fprintf( stderr, "libusb context should not initialized.\n" );
    }

    return 0;
}
