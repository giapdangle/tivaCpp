#include "tivaCppLibrary/include/Gpio.hpp"
#include "tivaCppLibrary/include/clock.hpp"
#include "tivaCppLibrary/delays.hpp"

#include "tivaCppLibrary/include/Uart.h"
#include "tivaCppLibrary/include/Core.hpp"
#include "tivaCppLibrary/interrupts.hpp"

#include "math.h"

#define HOST_STEP_TIME 0.025

double simTime = 0;

u8 buffTx[6] = {0,0,0,0,0,0};

double x1 = 0;
double x2 = 0;
double x3 = 0;

u16 x1_u16 = 0;
u16 x2_u16 = 0;
u16 x3_u16 = 0;

int main()
{
	clock::clock::config(clock::configOptions::clockSource::mainOscilator,
						 clock::configOptions::clockRate::clock_80Mhz);

	Gpio::enableClock(Gpio::peripheral::GPIOF);
	Gpio::enableClock(Gpio::peripheral::GPIOA);

	PF2::enableAsDigital();
	PF2::setMode(Gpio::options::mode::gpio);
	PF2::setIOmode(Gpio::options::IOmode::output);

	PA0::enableAsDigital();
	PA0::setMode(Gpio::options::mode::alternate);
	PA0::setAlternateMode(Gpio::options::altModes::alt1);

	PA1::enableAsDigital();
	PA1::setMode(Gpio::options::mode::alternate);
	PA1::setAlternateMode(Gpio::options::altModes::alt1);

	UART0::enableClock();
	UART0::configUart(uart::configOptions::baudrate::baud_1000000);

	core::IntEnableMaster();
	core::enableInterrupt(core::interrupts::uart0);

	while(1)
	{
		PF2::toogle();
		delays::delay_ms(250);
	}

}

void interruptFuncs::uart0rxtx_isr()
{
	UART0::clearInterrupts(uart::configOptions::interrupts::receiveInt);

	u8 foo = UART0::receiveByte();

	if ( foo == 's' )
	{
		// Do some stuff xD

		simTime += HOST_STEP_TIME;

		x1 = 0.5 + 0.5 * std::cos(simTime);
		x2 = 0.5 + 0.5 * std::sin(simTime);
		x3 = 0.5 + 0.5 * std::cos(simTime/2);

		x1 = ( x1 > 0.99 ) ? 0.99 : x1;
		x1 = ( x1 < 0.01 ) ? 0.01 : x1;

		x2 = ( x2 > 0.99 ) ? 0.99 : x2;
		x2 = ( x2 < 0.01 ) ? 0.01 : x2;

		x3 = ( x3 > 0.99 ) ? 0.99 : x3;
		x3 = ( x3 < 0.01 ) ? 0.01 : x3;

		x1_u16 = (u16)( x1 * 4095.0 );
		x2_u16 = (u16)( x2 * 4095.0 );
		x3_u16 = (u16)( x3 * 4095.0 );

		buffTx[0] = ( x1_u16 & 0xff00 ) >> 8;
		buffTx[1] = ( x1_u16 & 0x00ff ) >> 0;

		buffTx[2] = ( x2_u16 & 0xff00 ) >> 8;
		buffTx[3] = ( x2_u16 & 0x00ff ) >> 0;

		buffTx[4] = ( x3_u16 & 0xff00 ) >> 8;
		buffTx[5] = ( x3_u16 & 0x00ff ) >> 0;

		int i = 0;
		for( i = 0; i < 6; i++ )
		{
			UART0::sendByte( buffTx[i] );
		}
	}

}

