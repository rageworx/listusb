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
#include <cctype>
#include <vector>

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////

#define ME_STR              "listusb"
#define VERSION_STR         APP_VERSION_STR

#define SLEN_MANUFACTURER   128
#define SLEN_PRODUCT        128
#define SLEN_SN             64
#define SLEN_CLASS          64

////////////////////////////////////////////////////////////////////////////////

typedef struct _usbdevinfo {
    uint8_t     port;
    uint16_t    vid;
    uint16_t    pid;
    uint16_t    bcd;
    uint16_t    clsID[2];
    char        manufacturer[SLEN_MANUFACTURER];
    char        product[SLEN_PRODUCT];
    char        serialnumber[SLEN_SN];
    char        classname[SLEN_CLASS];
}usbdevdevinfo;

typedef struct _usbdevbusinfo {
    uint8_t                     bus;
    vector< usbdevdevinfo* >    device;
}usbdevbusinfo;

typedef vector< usbdevbusinfo* >  usbdevtree;

////////////////////////////////////////////////////////////////////////////////

static struct option long_opts[] = {
    { "help",           no_argument,        0, 'h' },
    { "simple",         no_argument,        0, 's' },
    { "reftable",       no_argument,        0, 'r' },
    { "version",        no_argument,        0, 'v' },
    { "color",          no_argument,        0, 'c' },
    { "lessinfo",       no_argument,        0, 'L' },
    { NULL, 0, 0, 0 }
};

static uint32_t         optpar_reftbl       = 0;
static uint32_t         optpar_simple       = 0;
static uint32_t         optpar_color        = 0;
static uint32_t         optpar_lessinfo     = 0;
static uint32_t         optpar_treeview     = 0;
static libusb_context*  libusbctx           = NULL;
static usbdevtree       usbtree;

////////////////////////////////////////////////////////////////////////////////

void trimStrInner( char *str )
{
    char*  spt = str;
    size_t len = strlen(str);
    size_t lsz = len;

    if ( len > 0 )
    {
        while( isspace(str[len - 1]) ) --len;
        while( *str && isspace(*str) ) ++str, --len;

        char* tmps = strdup( str );
        memset( spt, 0, lsz );
        memcpy( spt, tmps, len );
        free( tmps );
    }
}

void alloc_append_portdev( usbdevtree* udt, size_t bi, size_t l )
{
    if ( ( udt != NULL ) && ( l > 0 ) && ( bi < udt->size() ) )
    {
        for( size_t cnt=0; cnt<l; cnt++ )
        {
            usbdevdevinfo* ni = new usbdevdevinfo;
            if ( ni != NULL )
            {
                memset( ni, 0, sizeof( usbdevdevinfo ) );
                udt->at(bi)->device.push_back( ni );
            }
        }
    }
}

void alloc_append_businfo( usbdevtree* udt = NULL, size_t l = 0 )
{
    if ( ( udt != NULL ) && ( l > 0 ) )
    {
        for( size_t cnt=0; cnt<l; cnt++ )
        {
            usbdevbusinfo* ni = new usbdevbusinfo;
            if ( ni != NULL )
            {
                memset( ni, 0, sizeof( usbdevbusinfo ) );
                udt->push_back( ni );
            }
        }
    }
}

void free_portdev( usbdevtree& udt )
{
    if ( udt.size() > 0 )
    {
        for( size_t cnt=0; cnt<udt.size(); cnt++ )
        {
            if ( udt[cnt]->device.size() > 0 )
            {
                for( size_t itr=0; itr< udt[cnt]->device.size(); itr++ )
                {
                    usbdevdevinfo* prm = udt[cnt]->device[itr];
                    delete prm;
                    udt[cnt]->device[itr] = NULL;
                }

                udt[cnt]->device.clear();
            }

            usbdevbusinfo* prm = udt[cnt];
            delete prm;
            udt[cnt] = NULL;
        }
    }

    udt.clear();
}

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

void putUSBClass( usbdevdevinfo* pudi = NULL, uint8_t id = 0, uint8_t subid = 0 )
{
    if ( pudi != NULL )
    {
        pudi->clsID[0] = id;
        pudi->clsID[1] = subid;

        switch( id )
        {
            case LIBUSB_CLASS_PER_INTERFACE:
                snprintf( pudi->classname, SLEN_CLASS, "PER" );
                break;

            case LIBUSB_CLASS_AUDIO:
                snprintf( pudi->classname, SLEN_CLASS, "AUD." );
                break;

            case LIBUSB_CLASS_COMM:
                snprintf( pudi->classname, SLEN_CLASS, "COM." );
                break;

            case LIBUSB_CLASS_HID:
                snprintf( pudi->classname, SLEN_CLASS, "HID" );
                break;

            case LIBUSB_CLASS_PHYSICAL:
                snprintf( pudi->classname, SLEN_CLASS, "PHY." );
                break;

            case LIBUSB_CLASS_IMAGE:
                snprintf( pudi->classname, SLEN_CLASS, "IMG." );
                break;

            case LIBUSB_CLASS_PRINTER:
                snprintf( pudi->classname, SLEN_CLASS, "PRT." );
                break;

            case LIBUSB_CLASS_MASS_STORAGE:
                snprintf( pudi->classname, SLEN_CLASS, "MSD." );
                break;

            case LIBUSB_CLASS_HUB:
                snprintf( pudi->classname, SLEN_CLASS, "HUB" );
                break;

            case LIBUSB_CLASS_DATA:
                snprintf( pudi->classname, SLEN_CLASS, "DAT." );
                break;

            case LIBUSB_CLASS_SMART_CARD:
                snprintf( pudi->classname, SLEN_CLASS, "SCD." );
                break;

            case LIBUSB_CLASS_CONTENT_SECURITY:
                snprintf( pudi->classname, SLEN_CLASS, "CSD." );
                break;

            case LIBUSB_CLASS_VIDEO:
                snprintf( pudi->classname, SLEN_CLASS, "VID." );
                break;

            case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
                snprintf( pudi->classname, SLEN_CLASS, "PHD." );
                break;

            case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
                snprintf( pudi->classname, SLEN_CLASS, "DIA." );
                break;

            case LIBUSB_CLASS_WIRELESS:
                snprintf( pudi->classname, SLEN_CLASS, "WLS." );
                break;

            case LIBUSB_CLASS_MISCELLANEOUS:
                snprintf( pudi->classname, SLEN_CLASS, "MISC." );
                break;

            case LIBUSB_CLASS_APPLICATION:
                snprintf( pudi->classname, SLEN_CLASS, "APP." );
                break;

            case LIBUSB_CLASS_VENDOR_SPEC:
                snprintf( pudi->classname, SLEN_CLASS, "VSC" );
                break;

            default:
                snprintf( pudi->classname, SLEN_CLASS, "%04X", id );
                break;
        }
    }
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
        if ( optpar_lessinfo == 0 )
        {
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
                                    printf( "0x%02X", (uint8_t)*pE );
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
        } /// of if ( optpar_lessinfo == 0 )
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

            uint8_t dev_pn[SLEN_PRODUCT] = {0};
            uint8_t dev_mn[SLEN_MANUFACTURER] = {0};
            uint8_t dev_sn[SLEN_SN] = {0};

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
                                                        SLEN_PRODUCT );

                    libusb_get_string_descriptor_ascii( dev,
                                                  desc.iManufacturer,
                                                  dev_mn,
                                                  SLEN_MANUFACTURER );

                    libusb_get_string_descriptor_ascii( dev,
                                                  desc.iSerialNumber,
                                                  dev_sn,
                                                  SLEN_SN );

                    trimStrInner( (char*)dev_pn );
                    trimStrInner( (char*)dev_mn );
                    trimStrInner( (char*)dev_sn );
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
                        else
                        {
                            printf( "\n" );
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

size_t treelistdevs()
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
            usbdevdevinfo* curDevInfo = NULL;

            if ( libusb_get_device_descriptor( device, &desc ) == 0 )
            {
                uint8_t dev_bus = libusb_get_bus_number( device );
                uint8_t dev_port = libusb_get_port_number( device );

                if ( usbtree.size() == 0 )
                {
                    // first one
                    alloc_append_businfo( &usbtree, 1 );
                    usbtree[0]->bus = dev_bus;
                    alloc_append_portdev( &usbtree, 0, 1 );
                    curDevInfo = usbtree[0]->device[0];
                }
                else /// find same bus or not.
                {
                    bool foundBUS = false;
                    for ( size_t itr=0; itr<usbtree.size(); itr++ )
                    {
                        if ( usbtree[itr]->bus == dev_bus )
                        {
                            foundBUS = true;
                            alloc_append_portdev( &usbtree, itr, 1 );
                            size_t cq = usbtree[itr]->device.size();
                            if ( cq > 0 )
                                cq--;
                            curDevInfo = usbtree[itr]->device[cq];
                            break;
                        }
                    }

                    if ( foundBUS == false )
                    {
                        alloc_append_businfo( &usbtree, 1 );
                        alloc_append_portdev( &usbtree, usbtree.size()-1, 1 );
                        size_t cq = usbtree.size();
                        if ( cq > 0 )
                            cq--;
                        usbtree[cq]->bus = dev_bus;
                        curDevInfo = usbtree[usbtree.size()-1]->device[0];
                    }
                }

                if ( curDevInfo != NULL )
                {
                    curDevInfo->port = dev_port;
                    curDevInfo->vid  = desc.idVendor;
                    curDevInfo->pid  = desc.idProduct;
                }

                // open device ..
                int usberr = libusb_open( device, &dev );
                if ( usberr == 0 )
                {
                    libusb_get_string_descriptor_ascii( dev,
                                                        desc.iProduct,
                                                        (uint8_t*)curDevInfo->product,
                                                        SLEN_PRODUCT );

                    libusb_get_string_descriptor_ascii( dev,
                                                       desc.iManufacturer,
                                                       (uint8_t*)curDevInfo->manufacturer,
                                                       SLEN_MANUFACTURER );

                    libusb_get_string_descriptor_ascii( dev,
                                                        desc.iSerialNumber,
                                                        (uint8_t*)curDevInfo->serialnumber,
                                                        SLEN_SN );

                    if ( strlen( curDevInfo->product ) == 0 )
                    {
                        snprintf( curDevInfo->product, SLEN_PRODUCT, "-" );
                    }
                    else
                    {
                        trimStrInner( curDevInfo->product );
                    }

                    if ( strlen( curDevInfo->manufacturer ) == 0 )
                    {
                        snprintf( curDevInfo->manufacturer, SLEN_MANUFACTURER, "-" );
                    }
                    else
                    {
                        trimStrInner( curDevInfo->manufacturer );
                    }

                    if ( strlen( curDevInfo->serialnumber ) == 0 )
                    {
                        snprintf( curDevInfo->serialnumber, SLEN_SN, "-" );
                    }
                    else
                    {
                        trimStrInner( curDevInfo->serialnumber );
                    }
                }
                else
                {
                    dev = NULL;
                }

                putUSBClass( curDevInfo, desc.bDeviceClass, desc.bDeviceSubClass );
                curDevInfo->bcd = libusb_cpu_to_le16( desc.bcdUSB );

                if ( dev != NULL )
                    libusb_close( dev );
            }
        }

        for( size_t cnt=0; cnt<usbtree.size(); cnt++ )
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
            printf( "%03u", usbtree[cnt]->bus );

            if ( optpar_color > 0 )
            {
                printf( "\033[95m" );
            }
            printf( " : " );

            if ( optpar_color > 0 )
            {
                printf( "\033[92m" );
            }
            printf( "%zu devices\n", usbtree[cnt]->device.size() );

            for( size_t itr=0; itr<usbtree[cnt]->device.size(); itr++ )
            {
                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }
                printf( "  +-- " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[94m" );
                }
                printf( "Port " );

                if ( optpar_color > 0 )
                {
                    printf( "\033[97m" );
                }
                printf( "%03u ", usbtree[cnt]->device[itr]->port );

                if ( optpar_color > 0 )
                {
                    printf( "\033[92m" );
                }
                printf( "[%04X:%04X] ", usbtree[cnt]->device[itr]->vid,
                                        usbtree[cnt]->device[itr]->pid );

                // no need to decide color ...
                printf( "%s, ", bcd2human( usbtree[cnt]->device[itr]->bcd ) );

                if ( optpar_color > 0 )
                {
                    printf( "\033[93m" );
                }
                printf( "%s, ", usbtree[cnt]->device[itr]->classname );

                if ( optpar_color > 0 )
                {
                    printf( "\033[96m" );
                }
                printf( "%s, ", usbtree[cnt]->device[itr]->serialnumber );

                if ( optpar_color > 0 )
                {
                    printf( "\033[91m" );
                }
                printf( "%s, ", usbtree[cnt]->device[itr]->manufacturer );

                if ( optpar_color > 0 )
                {
                    printf( "\033[95m" );
                }
                printf( "%s", usbtree[cnt]->device[itr]->product );

                if ( optpar_color > 0 )
                {
                    printf( "\033[0m" );
                }
                printf( "\n" );
            }
        }

        free_portdev( usbtree );
    }

    return devscnt;
}

void showHelp()
{
    const char shortusage[] = \
"Usage: %s [-v] [-s] ... \n"
"\n"
"  -v,--version        display version only and quit.\n"
"  -s,--simple         display information as short as can.\n"
"  -c,--color          display with xterm-color escape codes.\n"
"  -r,--reftable       display reference table section with --simple.\n"
"  -L,--lessinfo       display information lesser than normal case.\n"
"  -t,--tree           display USB device tree ( not implemented )\n";

    fprintf( stdout, shortusage, ME_STR );
}

void showVersion()
{
    printf( "%s version %s, libusb version %d.%d.%d\n",
            ME_STR, VERSION_STR, LIBUSB_MAJOR, LIBUSB_MINOR, LIBUSB_MICRO );
}

int main( int argc, char** argv )
{
#ifdef __linux__
    int s_euid = geteuid();
    if ( s_euid > 10 )
    {
        fprintf( stderr, "WARNING: some linux not able to read correct USB information as normal user." );
        fprintf( stderr, " Use `sudo` to run %s to correct information if some informations are displayed as empty.\n",
                 ME_STR );
    }
#endif /// of __linux__

#ifdef DEBUG_LIBUSB
    putenv( "LIBUSB_DEBUG=4" );
#endif /// of DEBUG_LIBUSB

    // getopt
    for(;;)
    {
        int optidx = 0;
        int opt = getopt_long( argc, argv,
                               " :hvsctrL",
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
                    optpar_treeview = 1;
                    break;

                case 'r':
                    optpar_reftbl = 1;
                    break;

                case 'L':
                    optpar_lessinfo = 1;
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
            printf( "\033[97m" );
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
    libusb_init( &libusbctx );
#endif

    if ( libusbctx != NULL )
    {
        size_t devs = 0;

        if ( optpar_treeview == 0 )
        {
            devs = listdevs();
        }
        else
        {
            devs = treelistdevs();
        }

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
        else
        if ( ( devs == 0 ) && ( optpar_lessinfo == 0 ) )
        {
            if ( optpar_color > 0 )
            {
                printf( "\033[91m" );
            }

            printf( "no device found.\n" );

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
