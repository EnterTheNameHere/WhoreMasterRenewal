/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 Nuno Silva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <iostream>
#include <Rocket/Core.h>
#include "SystemInterfaceSFML.h"

int RocketSFMLSystemInterface::GetKeyModifiers()
{
	int Modifiers = 0;

	if( sf::Keyboard::isKeyPressed( sf::Keyboard::LShift ) ||
		sf::Keyboard::isKeyPressed( sf::Keyboard::RShift ))
		Modifiers |= Rocket::Core::Input::KM_SHIFT;

	if( sf::Keyboard::isKeyPressed( sf::Keyboard::LControl ) ||
		sf::Keyboard::isKeyPressed( sf::Keyboard::RControl ))
		Modifiers |= Rocket::Core::Input::KM_CTRL;

	if( sf::Keyboard::isKeyPressed( sf::Keyboard::LAlt ) ||
		sf::Keyboard::isKeyPressed( sf::Keyboard::RAlt ))
		Modifiers |= Rocket::Core::Input::KM_ALT;

	return Modifiers;
};

Rocket::Core::Input::KeyIdentifier RocketSFMLSystemInterface::TranslateKey( sf::Keyboard::Key key )
{
	switch( key )
	{
	case sf::Keyboard::Key::A:
		return Rocket::Core::Input::KI_A;
		break;
	case sf::Keyboard::Key::B:
		return Rocket::Core::Input::KI_B;
		break;
	case sf::Keyboard::Key::C:
		return Rocket::Core::Input::KI_C;
		break;
	case sf::Keyboard::Key::D:
		return Rocket::Core::Input::KI_D;
		break;
	case sf::Keyboard::Key::E:
		return Rocket::Core::Input::KI_E;
		break;
	case sf::Keyboard::Key::F:
		return Rocket::Core::Input::KI_F;
		break;
	case sf::Keyboard::Key::G:
		return Rocket::Core::Input::KI_G;
		break;
	case sf::Keyboard::Key::H:
		return Rocket::Core::Input::KI_H;
		break;
	case sf::Keyboard::Key::I:
		return Rocket::Core::Input::KI_I;
		break;
	case sf::Keyboard::Key::J:
		return Rocket::Core::Input::KI_J;
		break;
	case sf::Keyboard::Key::K:
		return Rocket::Core::Input::KI_K;
		break;
	case sf::Keyboard::Key::L:
		return Rocket::Core::Input::KI_L;
		break;
	case sf::Keyboard::Key::M:
		return Rocket::Core::Input::KI_M;
		break;
	case sf::Keyboard::Key::N:
		return Rocket::Core::Input::KI_N;
		break;
	case sf::Keyboard::Key::O:
		return Rocket::Core::Input::KI_O;
		break;
	case sf::Keyboard::Key::P:
		return Rocket::Core::Input::KI_P;
		break;
	case sf::Keyboard::Key::Q:
		return Rocket::Core::Input::KI_Q;
		break;
	case sf::Keyboard::Key::R:
		return Rocket::Core::Input::KI_R;
		break;
	case sf::Keyboard::Key::S:
		return Rocket::Core::Input::KI_S;
		break;
	case sf::Keyboard::Key::T:
		return Rocket::Core::Input::KI_T;
		break;
	case sf::Keyboard::Key::U:
		return Rocket::Core::Input::KI_U;
		break;
	case sf::Keyboard::Key::V:
		return Rocket::Core::Input::KI_V;
		break;
	case sf::Keyboard::Key::W:
		return Rocket::Core::Input::KI_W;
		break;
	case sf::Keyboard::Key::X:
		return Rocket::Core::Input::KI_X;
		break;
	case sf::Keyboard::Key::Y:
		return Rocket::Core::Input::KI_Y;
		break;
	case sf::Keyboard::Key::Z:
		return Rocket::Core::Input::KI_Z;
		break;
	case sf::Keyboard::Key::Num0:
		return Rocket::Core::Input::KI_0;
		break;
	case sf::Keyboard::Key::Num1:
		return Rocket::Core::Input::KI_1;
		break;
	case sf::Keyboard::Key::Num2:
		return Rocket::Core::Input::KI_2;
		break;
	case sf::Keyboard::Key::Num3:
		return Rocket::Core::Input::KI_3;
		break;
	case sf::Keyboard::Key::Num4:
		return Rocket::Core::Input::KI_4;
		break;
	case sf::Keyboard::Key::Num5:
		return Rocket::Core::Input::KI_5;
		break;
	case sf::Keyboard::Key::Num6:
		return Rocket::Core::Input::KI_6;
		break;
	case sf::Keyboard::Key::Num7:
		return Rocket::Core::Input::KI_7;
		break;
	case sf::Keyboard::Key::Num8:
		return Rocket::Core::Input::KI_8;
		break;
	case sf::Keyboard::Key::Num9:
		return Rocket::Core::Input::KI_9;
		break;
	case sf::Keyboard::Key::Numpad0:
		return Rocket::Core::Input::KI_NUMPAD0;
		break;
	case sf::Keyboard::Key::Numpad1:
		return Rocket::Core::Input::KI_NUMPAD1;
		break;
	case sf::Keyboard::Key::Numpad2:
		return Rocket::Core::Input::KI_NUMPAD2;
		break;
	case sf::Keyboard::Key::Numpad3:
		return Rocket::Core::Input::KI_NUMPAD3;
		break;
	case sf::Keyboard::Key::Numpad4:
		return Rocket::Core::Input::KI_NUMPAD4;
		break;
	case sf::Keyboard::Key::Numpad5:
		return Rocket::Core::Input::KI_NUMPAD5;
		break;
	case sf::Keyboard::Key::Numpad6:
		return Rocket::Core::Input::KI_NUMPAD6;
		break;
	case sf::Keyboard::Key::Numpad7:
		return Rocket::Core::Input::KI_NUMPAD7;
		break;
	case sf::Keyboard::Key::Numpad8:
		return Rocket::Core::Input::KI_NUMPAD8;
		break;
	case sf::Keyboard::Key::Numpad9:
		return Rocket::Core::Input::KI_NUMPAD9;
		break;
	case sf::Keyboard::Key::Left:
		return Rocket::Core::Input::KI_LEFT;
		break;
	case sf::Keyboard::Key::Right:
		return Rocket::Core::Input::KI_RIGHT;
		break;
	case sf::Keyboard::Key::Up:
		return Rocket::Core::Input::KI_UP;
		break;
	case sf::Keyboard::Key::Down:
		return Rocket::Core::Input::KI_DOWN;
		break;
	case sf::Keyboard::Key::Add:
		return Rocket::Core::Input::KI_ADD;
		break;
	case sf::Keyboard::Key::BackSpace:
		return Rocket::Core::Input::KI_BACK;
		break;
	case sf::Keyboard::Key::Delete:
		return Rocket::Core::Input::KI_DELETE;
		break;
	case sf::Keyboard::Key::Divide:
		return Rocket::Core::Input::KI_DIVIDE;
		break;
	case sf::Keyboard::Key::End:
		return Rocket::Core::Input::KI_END;
		break;
	case sf::Keyboard::Key::Escape:
		return Rocket::Core::Input::KI_ESCAPE;
		break;
	case sf::Keyboard::Key::F1:
		return Rocket::Core::Input::KI_F1;
		break;
	case sf::Keyboard::Key::F2:
		return Rocket::Core::Input::KI_F2;
		break;
	case sf::Keyboard::Key::F3:
		return Rocket::Core::Input::KI_F3;
		break;
	case sf::Keyboard::Key::F4:
		return Rocket::Core::Input::KI_F4;
		break;
	case sf::Keyboard::Key::F5:
		return Rocket::Core::Input::KI_F5;
		break;
	case sf::Keyboard::Key::F6:
		return Rocket::Core::Input::KI_F6;
		break;
	case sf::Keyboard::Key::F7:
		return Rocket::Core::Input::KI_F7;
		break;
	case sf::Keyboard::Key::F8:
		return Rocket::Core::Input::KI_F8;
		break;
	case sf::Keyboard::Key::F9:
		return Rocket::Core::Input::KI_F9;
		break;
	case sf::Keyboard::Key::F10:
		return Rocket::Core::Input::KI_F10;
		break;
	case sf::Keyboard::Key::F11:
		return Rocket::Core::Input::KI_F11;
		break;
	case sf::Keyboard::Key::F12:
		return Rocket::Core::Input::KI_F12;
		break;
	case sf::Keyboard::Key::F13:
		return Rocket::Core::Input::KI_F13;
		break;
	case sf::Keyboard::Key::F14:
		return Rocket::Core::Input::KI_F14;
		break;
	case sf::Keyboard::Key::F15:
		return Rocket::Core::Input::KI_F15;
		break;
	case sf::Keyboard::Key::Home:
		return Rocket::Core::Input::KI_HOME;
		break;
	case sf::Keyboard::Key::Insert:
		return Rocket::Core::Input::KI_INSERT;
		break;
	case sf::Keyboard::Key::LControl:
		return Rocket::Core::Input::KI_LCONTROL;
		break;
	case sf::Keyboard::Key::LShift:
		return Rocket::Core::Input::KI_LSHIFT;
		break;
	case sf::Keyboard::Key::Multiply:
		return Rocket::Core::Input::KI_MULTIPLY;
		break;
	case sf::Keyboard::Key::Pause:
		return Rocket::Core::Input::KI_PAUSE;
		break;
	case sf::Keyboard::Key::RControl:
		return Rocket::Core::Input::KI_RCONTROL;
		break;
	case sf::Keyboard::Key::Return:
		return Rocket::Core::Input::KI_RETURN;
		break;
	case sf::Keyboard::Key::RShift:
		return Rocket::Core::Input::KI_RSHIFT;
		break;
	case sf::Keyboard::Key::Space:
		return Rocket::Core::Input::KI_SPACE;
		break;
	case sf::Keyboard::Key::Subtract:
		return Rocket::Core::Input::KI_SUBTRACT;
		break;
	case sf::Keyboard::Key::Tab:
		return Rocket::Core::Input::KI_TAB;
		break;
    default:
        return Rocket::Core::Input::KI_UNKNOWN;
        break;
	};

	return Rocket::Core::Input::KI_UNKNOWN;
};

float RocketSFMLSystemInterface::GetElapsedTime()
{
	return timer.getElapsedTime().asMilliseconds();
};

bool RocketSFMLSystemInterface::LogMessage( Rocket::Core::Log::Type type, const Rocket::Core::String& message )
{
	std::string typeOfMessagePrefix;

	switch( type )
	{
	case Rocket::Core::Log::LT_ALWAYS:
		typeOfMessagePrefix = "[Always]";
		break;
	case Rocket::Core::Log::LT_ERROR:
		typeOfMessagePrefix = "[Error]";
		break;
	case Rocket::Core::Log::LT_ASSERT:
		typeOfMessagePrefix = "[Assert]";
		break;
	case Rocket::Core::Log::LT_WARNING:
		typeOfMessagePrefix = "[Warning]";
		break;
	case Rocket::Core::Log::LT_INFO:
		typeOfMessagePrefix = "[Info]";
		break;
	case Rocket::Core::Log::LT_DEBUG:
		typeOfMessagePrefix = "[Debug]";
		break;
    default:
        typeOfMessagePrefix = "[Unknown]";
        break;
	};
    
    std::cout << typeOfMessagePrefix << " - " << message.CString() << "\n";
    
	//printf( "%s - %s\n", typeOfMessagePrefix.c_str(), message.CString() );
    
	return true;
};
