/**
 * @file uwbpacketclass.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of uwbPacketClass class methods.
 *
 * @section DESCRIPTION
 *
 * Data in UWB sensor network are being sent into central unit via wireless connection. Central unit has
 * device (modul) for recieving such communication and sending recieved bytes through serial link into
 * computer. The software must be able read and write into such packets as well as send them via RS232.
 *
 * This class provides functionality for both, recieving and sending packets. You can comment one of that
 * functionality to reduce the size of compiled program if you do not need it.
 *
 */

#include "uwbpacketclass.h"

/*****************************************************************************************************************/

uwbPacketTx::uwbPacketTx(int radarId, int port) : endingChar('$'), rounder(100.0)
{
    radarID = radarId;
    packetCount = 0;
    radarTime = 0;
    comPort = port;

    packet = NULL;
    packetLength = 0;

    crc_tab16_init = FALSE;
}

void uwbPacketTx::generatePacket(float *data, int data_count)
{

    // allocate enough array space
    deleteLastPacket();
    packet = new unsigned char[data_count*2*3+5];

    std::bitset<8> b_radarID(radarID);
    std::bitset<8> b_radarTime(radarTime);
    std::bitset<8> b_packetNumber(packetCount);

    std::bitset<8> b_constant(15);

    unsigned char ch; // temp char

    // convert radarID
    ch = makeCorrection(((b_radarID>>4)&b_constant).to_ulong());
    packet[packetLength++] = ch;
    ch = makeCorrection((b_radarID&b_constant).to_ulong());
    packet[packetLength++] = ch;

    // convert radarTime
    ch = makeCorrection(((b_radarTime>>4)&b_constant).to_ulong());
    packet[packetLength++] = ch;
    ch = makeCorrection((b_radarTime&b_constant).to_ulong());
    packet[packetLength++] = ch;

    // convert packetCount
    ch = makeCorrection(((b_packetNumber>>4)&b_constant).to_ulong());
    packet[packetLength++] = ch;
    ch = makeCorrection((b_packetNumber&b_constant).to_ulong());
    packet[packetLength++] = ch;

    std::bitset<12> b_val_constant(15); // used for 4 lowest bits extraction
    bool sign; // indicates if number is negative (false), positive (true)
    for(int i=0; i<data_count; i++)
    {
        // convert float to int
        int num = data[i]*rounder;

        // check if value is negative
        sign = false;
        (num<0) ? num*=(-1) : sign=true;

        // if absolute value of number is greater than 2047, this is more than we can handle on 11 bits, so its rounded to 2047
        if(num>=2048) sign ? num=2047 : num=2048;
        // prepare number for two's complement
        if(!sign && num!=0) --num;

        std::bitset<12> b_temp(num);

        // convert x and y to 3 chars since values can be passed as 2x4 bits + 1x3 bit long + 1 sign bit = 12 bits
        for(int j=8; j>=0; j=j-4)
        {
            if(j==8) // extract top 3 bits
            {
                ch = ((sign ? (b_temp>>j) : (b_temp>>j).flip())&b_val_constant).to_ulong();
                ch = makeCorrection(ch);
            }
            else // other bits
                ch = makeCorrection(((sign ? (b_temp>>j) : (b_temp>>j).flip())&b_val_constant).to_ulong());

            packet[packetLength++] = ch;
        }
    }

    // calculate CRC
    unsigned short crc = 0;
    crc_tab16_init = FALSE;
    for(int k = 0; k<packetLength; k++)
        crc = update_crc_16(crc, packet[k]);

    std::bitset<16> b_crc(crc);

    std::bitset<16> crc_constant(15); // constant for extraction 4 bits from 16 bit long bitstream
    for(int j=12; j>=0; j=j-4)
    {
        ch = makeCorrection(((b_crc>>j)&crc_constant).to_ulong());
        packet[packetLength++] = ch;
    }
}

unsigned char uwbPacketTx::makeCorrection(unsigned char ch)
{
    int ascii = (int)(ch)+48;

    if(ascii>57)
    {
        ascii += 7;
    }

    return (unsigned char)(ascii);
}

unsigned short uwbPacketTx::update_crc_16( unsigned short crc, char c )
{

    unsigned short tmp, short_c;

    short_c = 0x00ff & (unsigned short) c;

    if ( ! crc_tab16_init ) this->init_crc16_tab();

    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

    return crc;

}

void uwbPacketTx::init_crc16_tab()
{
        int i, j;
        unsigned short crc, c;

        for (i=0; i<256; i++) {

            crc = 0;
            c   = (unsigned short) i;

            for (j=0; j<8; j++) {

                if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ P_16;
                else                      crc =   crc >> 1;

                c = c >> 1;
            }

            crc_tab16[i] = crc;
        }

        crc_tab16_init = TRUE;
}

void uwbPacketTx::deleteLastPacket()
{
    if(packet!=NULL)
    {
        delete packet;
        packet = NULL;
    }
    packetLength = 0;
}

bool uwbPacketTx::sendPacket()
{
    // if there is no packet to send
    if(packet==NULL) return false;

    int bytes_sent_error;
    for(int i = 0; i<packetLength; i++)
    {
        bytes_sent_error = RS232_SendByte(comPort, packet[i]);
        if(bytes_sent_error) break;
    }

    if(!bytes_sent_error)
    {
        RS232_SendByte(comPort, endingChar); // if no error occured, send the ending char
        incrementPacketCount(); // since now, this function cannot know if something is wrong and it can increment its packet count/number of packets successfuly sent
    }

    return (bytes_sent_error>0 ? true : false);
}

void uwbPacketTx::incrementPacketCount()
{
    ++packetCount;
    // packet count sending in packet must be 8 bit long, maximum is therefore 255
    if(packetCount>255) packetCount = 0;
}

void uwbPacketTx::decrementPacketCount()
{
    --packetCount;
    // packet count sending in packet must be 8 bit long, maximum is therefore 255
    if(packetCount<0) packetCount = 255;
}

/**********************************************************************************************************************/


uwbPacketRx::uwbPacketRx(int port) : endingChar('$'), rounder(100.0), buffer_size(128), c_buffer_size(128)
{
    comPort = port;
    radarID = radarTime = packetCount = dataCount = 0;
    data = NULL;
    packet = NULL;

    crc_tab16_init = FALSE;

    buffer_read_pointer = buffer_stack_pointer = buffer_rs232_read_size = buffer_packet_size = packetLength = 0;

    buffer = new unsigned char[buffer_size];
    c_buffer = new unsigned char[c_buffer_size];
}

int uwbPacketRx::readPacket()
{
    // check basic bytes count to detect some simple errors

    // since overhead is 10 bytes, if packet has less than that, obviously some error occured
    if(packetLength<11) return -2;
    // if data part modulo 6 is greater than 0, some data are missing
    else if((packetLength-10)%6) return -1;

    int ch;
    int stack_pointer = 0;

    std::bitset<16> crc_temp(0);

    for(int i=(packetLength-4); i<packetLength; i++)
    {
        crc_temp <<= 4;

        // read 4 bits
        ch = (int)(removeCorrection(packet[i]));

        crc_temp |= ch;
    }

    unsigned short crc_read = crc_temp.to_ulong();

    // go through characters and check if the calculated and read crc will match
    // this is necessary to be done before data are read because we will not modify existing variables
    // with incorrect data in case the crc is wrong

    unsigned short crc_calc = 0;
    crc_tab16_init = FALSE;
    for(int k = 0; k<(packetLength-4); k++)
        crc_calc = update_crc_16(crc_calc, packet[k]);

    // if crc does not match
    if(crc_read != crc_calc) return 0;

    std::bitset<8> b_temp(0);
    std::bitset<12> val_temp(0);

    // read radar ID
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    b_temp <<= 4;
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    radarID = b_temp.to_ulong();
    b_temp &= 0; // zero all bits

    // read radar Time
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    b_temp <<= 4;
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    radarTime = b_temp.to_ulong();
    b_temp &= 0; // zero all bits

    // read packet Count
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    b_temp <<= 4;
    ch = (int)(removeCorrection(packet[stack_pointer++]));
    b_temp |= ch;
    packetCount = b_temp.to_ulong();
    b_temp &= 0; // zero all bits

    // go throught all availible data
    int internal_counter = 0; // if reaches 3, we have all 12 bits, therefore we have one value
    int value_counter = 0; // index of buffer

    // create data array as long as needed
    dataCount = packetLength-10; // packet length contains CRC, radarID, radarTime, packetCount bytes

    // if MTT_ARRAY_FIT macro is 1 then we want to allocate array for maximum allowable coordinates possible, not exactly for recieved coordinates
    #if defined MTT_ARRAY_FIT && MTT_ARRAY_FIT==1
        data = new float[MAX_N*2];
    #else
        data = new float[dataCount];
    #endif

    dataCount /= 3; // since real data are represented by 3 chars, real data count is 3 times smaller as character count

    while(stack_pointer<(packetLength-4)) // last four bytes are not values, but CRC. StackPointer holds index position!
    {
        // read 4 bits
        ch = (int)(removeCorrection(packet[stack_pointer++]));

        val_temp |= ch;

        if(++internal_counter<3)
            val_temp <<= 4; // if counter is less than 3 we still need some bits to complete value
        else
        {
            // all bits are read
            internal_counter = 0;
            // if sign bit is set to 1, need to create negative number
            bool sign = val_temp[11];
            data[value_counter++] = (sign ? (-1.0)*((float)(val_temp.flip().to_ulong())+1.0)/rounder : ((float)(val_temp.to_ulong()))/rounder);

            val_temp &= 0; // reset temporary bit stream
        }
    }
}

void uwbPacketRx::deleteLastPacket()
{
    if(packet!=NULL) delete packet;
    packet = NULL;
}

bool uwbPacketRx::recievePacket()
{
    // read data from serial link
    buffer_rs232_read_size = RS232_PollComport(comPort, buffer, buffer_size);

    // save data into second buffer
    for(int i=0; i<buffer_rs232_read_size; i++)
    {
        c_buffer[buffer_stack_pointer++] = buffer[i];
        if(buffer_stack_pointer>=c_buffer_size) buffer_stack_pointer = 0; // close cyclic buffer
    }

    // try to find ending char
    while(buffer_read_pointer!=buffer_stack_pointer)
    {
        if(c_buffer[buffer_read_pointer++]==endingChar)
        {
            if(buffer_read_pointer>=c_buffer_size) buffer_read_pointer = 0; // close cyclic buffer

            // if character is ending char, we need to save packet
            // buffer_packet_size is not incremented so endingChar is not countet into packet char count
            deleteLastPacket();
            packet = new unsigned char[buffer_packet_size];
            // copy characters to packet array

            buffer_read_pointer -= buffer_packet_size+1;
            if(buffer_read_pointer<0) buffer_read_pointer += c_buffer_size;
            for(int i=0; i<buffer_packet_size; i++)
            {
                packet[i] = c_buffer[buffer_read_pointer++];
                if(buffer_read_pointer>=c_buffer_size) buffer_read_pointer = 0; // close cyclic buffer
            }

            buffer_read_pointer++; // return buffer read pointer back to initiall position

            if(buffer_read_pointer>=c_buffer_size) buffer_read_pointer = 0; // close cyclic buffer

            packetLength = buffer_packet_size; // save packet size in case that higher functions will want to do some processing on packet string
            buffer_packet_size = 0; // zero size, so next time we can count again

            return true; // inform higher functions about packet was successfully read
        }
        else
        {
            // if character is not ending char, continue reading chars
            buffer_packet_size++;
        }

        if(buffer_read_pointer>=c_buffer_size) buffer_read_pointer = 0; // close cyclic buffer
    }


    return false; // packet not read or is not complete
}

int uwbPacketRx::removeCorrection(unsigned char ch)
{
    int ascii = (int)(ch);
    if(ascii>64) ascii-=7;

    ascii -= 48;

    return ascii;
}

unsigned short uwbPacketRx::update_crc_16( unsigned short crc, char c )
{

    unsigned short tmp, short_c;

    short_c = 0x00ff & (unsigned short) c;

    if ( ! crc_tab16_init ) this->init_crc16_tab();

    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

    return crc;

}

void uwbPacketRx::init_crc16_tab()
{
        int i, j;
        unsigned short crc, c;

        for (i=0; i<256; i++) {

            crc = 0;
            c   = (unsigned short) i;

            for (j=0; j<8; j++) {

                if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ P_16;
                else                      crc =   crc >> 1;

                c = c >> 1;
            }

            crc_tab16[i] = crc;
        }

        crc_tab16_init = TRUE;
}
