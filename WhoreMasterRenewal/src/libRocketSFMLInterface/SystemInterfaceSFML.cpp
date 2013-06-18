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

#include "SystemInterfaceSFML.h"

#include <Rocket/Core.h>

#include <iostream>

int RocketSFMLSystemInterface::GetKeyModifiers( sf::Event& event )
{
    int Modifiers = 0;
    
    if( event.key.shift )
        Modifiers |= Rocket::Core::Input::KeyModifier::KM_SHIFT;
        
    if( event.key.control )
        Modifiers |= Rocket::Core::Input::KeyModifier::KM_CTRL;
        
    if( event.key.alt )
        Modifiers |= Rocket::Core::Input::KeyModifier::KM_ALT;
        
    return Modifiers;
}

Rocket::Core::Input::KeyIdentifier RocketSFMLSystemInterface::TranslateKey( sf::Keyboard::Key key )
{
    switch( key )
    {
    case sf::Keyboard::Key::A:
        return Rocket::Core::Input::KeyIdentifier::KI_A;
        break;
        
    case sf::Keyboard::Key::B:
        return Rocket::Core::Input::KeyIdentifier::KI_B;
        break;
        
    case sf::Keyboard::Key::C:
        return Rocket::Core::Input::KeyIdentifier::KI_C;
        break;
        
    case sf::Keyboard::Key::D:
        return Rocket::Core::Input::KeyIdentifier::KI_D;
        break;
        
    case sf::Keyboard::Key::E:
        return Rocket::Core::Input::KeyIdentifier::KI_E;
        break;
        
    case sf::Keyboard::Key::F:
        return Rocket::Core::Input::KeyIdentifier::KI_F;
        break;
        
    case sf::Keyboard::Key::G:
        return Rocket::Core::Input::KeyIdentifier::KI_G;
        break;
        
    case sf::Keyboard::Key::H:
        return Rocket::Core::Input::KeyIdentifier::KI_H;
        break;
        
    case sf::Keyboard::Key::I:
        return Rocket::Core::Input::KeyIdentifier::KI_I;
        break;
        
    case sf::Keyboard::Key::J:
        return Rocket::Core::Input::KeyIdentifier::KI_J;
        break;
        
    case sf::Keyboard::Key::K:
        return Rocket::Core::Input::KeyIdentifier::KI_K;
        break;
        
    case sf::Keyboard::Key::L:
        return Rocket::Core::Input::KeyIdentifier::KI_L;
        break;
        
    case sf::Keyboard::Key::M:
        return Rocket::Core::Input::KeyIdentifier::KI_M;
        break;
        
    case sf::Keyboard::Key::N:
        return Rocket::Core::Input::KeyIdentifier::KI_N;
        break;
        
    case sf::Keyboard::Key::O:
        return Rocket::Core::Input::KeyIdentifier::KI_O;
        break;
        
    case sf::Keyboard::Key::P:
        return Rocket::Core::Input::KeyIdentifier::KI_P;
        break;
        
    case sf::Keyboard::Key::Q:
        return Rocket::Core::Input::KeyIdentifier::KI_Q;
        break;
        
    case sf::Keyboard::Key::R:
        return Rocket::Core::Input::KeyIdentifier::KI_R;
        break;
        
    case sf::Keyboard::Key::S:
        return Rocket::Core::Input::KeyIdentifier::KI_S;
        break;
        
    case sf::Keyboard::Key::T:
        return Rocket::Core::Input::KeyIdentifier::KI_T;
        break;
        
    case sf::Keyboard::Key::U:
        return Rocket::Core::Input::KeyIdentifier::KI_U;
        break;
        
    case sf::Keyboard::Key::V:
        return Rocket::Core::Input::KeyIdentifier::KI_V;
        break;
        
    case sf::Keyboard::Key::W:
        return Rocket::Core::Input::KeyIdentifier::KI_W;
        break;
        
    case sf::Keyboard::Key::X:
        return Rocket::Core::Input::KeyIdentifier::KI_X;
        break;
        
    case sf::Keyboard::Key::Y:
        return Rocket::Core::Input::KeyIdentifier::KI_Y;
        break;
        
    case sf::Keyboard::Key::Z:
        return Rocket::Core::Input::KeyIdentifier::KI_Z;
        break;
        
    case sf::Keyboard::Key::Num0:
        return Rocket::Core::Input::KeyIdentifier::KI_0;
        break;
        
    case sf::Keyboard::Key::Num1:
        return Rocket::Core::Input::KeyIdentifier::KI_1;
        break;
        
    case sf::Keyboard::Key::Num2:
        return Rocket::Core::Input::KeyIdentifier::KI_2;
        break;
        
    case sf::Keyboard::Key::Num3:
        return Rocket::Core::Input::KeyIdentifier::KI_3;
        break;
        
    case sf::Keyboard::Key::Num4:
        return Rocket::Core::Input::KeyIdentifier::KI_4;
        break;
        
    case sf::Keyboard::Key::Num5:
        return Rocket::Core::Input::KeyIdentifier::KI_5;
        break;
        
    case sf::Keyboard::Key::Num6:
        return Rocket::Core::Input::KeyIdentifier::KI_6;
        break;
        
    case sf::Keyboard::Key::Num7:
        return Rocket::Core::Input::KeyIdentifier::KI_7;
        break;
        
    case sf::Keyboard::Key::Num8:
        return Rocket::Core::Input::KeyIdentifier::KI_8;
        break;
        
    case sf::Keyboard::Key::Num9:
        return Rocket::Core::Input::KeyIdentifier::KI_9;
        break;
        
    case sf::Keyboard::Key::Numpad0:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD0;
        break;
        
    case sf::Keyboard::Key::Numpad1:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD1;
        break;
        
    case sf::Keyboard::Key::Numpad2:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD2;
        break;
        
    case sf::Keyboard::Key::Numpad3:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD3;
        break;
        
    case sf::Keyboard::Key::Numpad4:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD4;
        break;
        
    case sf::Keyboard::Key::Numpad5:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD5;
        break;
        
    case sf::Keyboard::Key::Numpad6:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD6;
        break;
        
    case sf::Keyboard::Key::Numpad7:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD7;
        break;
        
    case sf::Keyboard::Key::Numpad8:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD8;
        break;
        
    case sf::Keyboard::Key::Numpad9:
        return Rocket::Core::Input::KeyIdentifier::KI_NUMPAD9;
        break;
        
    case sf::Keyboard::Key::Left:
        return Rocket::Core::Input::KeyIdentifier::KI_LEFT;
        break;
        
    case sf::Keyboard::Key::Right:
        return Rocket::Core::Input::KeyIdentifier::KI_RIGHT;
        break;
        
    case sf::Keyboard::Key::Up:
        return Rocket::Core::Input::KeyIdentifier::KI_UP;
        break;
        
    case sf::Keyboard::Key::Down:
        return Rocket::Core::Input::KeyIdentifier::KI_DOWN;
        break;
        
    case sf::Keyboard::Key::Add:
        return Rocket::Core::Input::KeyIdentifier::KI_ADD;
        break;
        
    case sf::Keyboard::Key::BackSpace:
        return Rocket::Core::Input::KeyIdentifier::KI_BACK;
        break;
        
    case sf::Keyboard::Key::Delete:
        return Rocket::Core::Input::KeyIdentifier::KI_DELETE;
        break;
        
    case sf::Keyboard::Key::Divide:
        return Rocket::Core::Input::KeyIdentifier::KI_DIVIDE;
        break;
        
    case sf::Keyboard::Key::End:
        return Rocket::Core::Input::KeyIdentifier::KI_END;
        break;
        
    case sf::Keyboard::Key::Escape:
        return Rocket::Core::Input::KeyIdentifier::KI_ESCAPE;
        break;
        
    case sf::Keyboard::Key::F1:
        return Rocket::Core::Input::KeyIdentifier::KI_F1;
        break;
        
    case sf::Keyboard::Key::F2:
        return Rocket::Core::Input::KeyIdentifier::KI_F2;
        break;
        
    case sf::Keyboard::Key::F3:
        return Rocket::Core::Input::KeyIdentifier::KI_F3;
        break;
        
    case sf::Keyboard::Key::F4:
        return Rocket::Core::Input::KeyIdentifier::KI_F4;
        break;
        
    case sf::Keyboard::Key::F5:
        return Rocket::Core::Input::KeyIdentifier::KI_F5;
        break;
        
    case sf::Keyboard::Key::F6:
        return Rocket::Core::Input::KeyIdentifier::KI_F6;
        break;
        
    case sf::Keyboard::Key::F7:
        return Rocket::Core::Input::KeyIdentifier::KI_F7;
        break;
        
    case sf::Keyboard::Key::F8:
        return Rocket::Core::Input::KeyIdentifier::KI_F8;
        break;
        
    case sf::Keyboard::Key::F9:
        return Rocket::Core::Input::KeyIdentifier::KI_F9;
        break;
        
    case sf::Keyboard::Key::F10:
        return Rocket::Core::Input::KeyIdentifier::KI_F10;
        break;
        
    case sf::Keyboard::Key::F11:
        return Rocket::Core::Input::KeyIdentifier::KI_F11;
        break;
        
    case sf::Keyboard::Key::F12:
        return Rocket::Core::Input::KeyIdentifier::KI_F12;
        break;
        
    case sf::Keyboard::Key::F13:
        return Rocket::Core::Input::KeyIdentifier::KI_F13;
        break;
        
    case sf::Keyboard::Key::F14:
        return Rocket::Core::Input::KeyIdentifier::KI_F14;
        break;
        
    case sf::Keyboard::Key::F15:
        return Rocket::Core::Input::KeyIdentifier::KI_F15;
        break;
        
    case sf::Keyboard::Key::Home:
        return Rocket::Core::Input::KeyIdentifier::KI_HOME;
        break;
        
    case sf::Keyboard::Key::Insert:
        return Rocket::Core::Input::KeyIdentifier::KI_INSERT;
        break;
        
    case sf::Keyboard::Key::LControl:
        return Rocket::Core::Input::KeyIdentifier::KI_LCONTROL;
        break;
        
    case sf::Keyboard::Key::LShift:
        return Rocket::Core::Input::KeyIdentifier::KI_LSHIFT;
        break;
        
    case sf::Keyboard::Key::Multiply:
        return Rocket::Core::Input::KeyIdentifier::KI_MULTIPLY;
        break;
        
    case sf::Keyboard::Key::Pause:
        return Rocket::Core::Input::KeyIdentifier::KI_PAUSE;
        break;
        
    case sf::Keyboard::Key::RControl:
        return Rocket::Core::Input::KeyIdentifier::KI_RCONTROL;
        break;
        
    case sf::Keyboard::Key::Return:
        return Rocket::Core::Input::KeyIdentifier::KI_RETURN;
        break;
        
    case sf::Keyboard::Key::RShift:
        return Rocket::Core::Input::KeyIdentifier::KI_RSHIFT;
        break;
        
    case sf::Keyboard::Key::Space:
        return Rocket::Core::Input::KeyIdentifier::KI_SPACE;
        break;
        
    case sf::Keyboard::Key::Subtract:
        return Rocket::Core::Input::KeyIdentifier::KI_SUBTRACT;
        break;
        
    case sf::Keyboard::Key::Tab:
        return Rocket::Core::Input::KeyIdentifier::KI_TAB;
        break;
        
    case sf::Keyboard::Key::LSystem:
        return Rocket::Core::Input::KeyIdentifier::KI_LWIN;
        break;
        
    case sf::Keyboard::Key::RSystem:
        return Rocket::Core::Input::KeyIdentifier::KI_RWIN;
        break;
        
    case sf::Keyboard::Key::Menu:
        return Rocket::Core::Input::KeyIdentifier::KI_LMENU; // possible RMENU too...
        break;
        
        // no equivalent found (yet)
    case sf::Keyboard::Key::LAlt:
        std::cout << "No equivalent Rocket key: LAlt\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::RAlt:
        std::cout << "No equivalent Rocket key: RAlt\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::LBracket:
        std::cout << "No equivalent Rocket key: LBracket\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::RBracket:
        std::cout << "No equivalent Rocket key: RBracket\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::SemiColon:
        std::cout << "No equivalent Rocket key: SemiColon\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Comma:
        std::cout << "No equivalent Rocket key: Comma\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Period:
        std::cout << "No equivalent Rocket key: Period\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Quote:
        std::cout << "No equivalent Rocket key: Quote\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Slash:
        std::cout << "No equivalent Rocket key: Slash\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::BackSlash:
        std::cout << "No equivalent Rocket key: BackSlash\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Tilde:
        std::cout << "No equivalent Rocket key: Tilde\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Equal:
        std::cout << "No equivalent Rocket key: Equal\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::Dash:
        std::cout << "No equivalent Rocket key: Dash\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::PageUp:
        std::cout << "No equivalent Rocket key: PageUp\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
    case sf::Keyboard::Key::PageDown:
        std::cout << "No equivalent Rocket key: PageDown\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
        // unknown key
    case sf::Keyboard::Key::Unknown:
        std::cout << "Unknown key\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
        // this value stores number of keys in sf key enumeration, so ignore
    case sf::Keyboard::Key::KeyCount:
        std::cout << "KeyCount\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
        
        // report default case as Unknown key too
    default:
        std::cout << "default: keycode=" << static_cast<long int>( key ) << "\n";
        return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;
        break;
    };
}

float RocketSFMLSystemInterface::GetElapsedTime()
{
    return static_cast<float>( timer.getElapsedTime().asMilliseconds() );
}

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
        
        // this value stores maximum number in enumeration, so ignore
    case Rocket::Core::Log::Type::LT_MAX:
    default:
        typeOfMessagePrefix = "[Unknown]";
        break;
    };
    
    std::cout << typeOfMessagePrefix << " - " << message.CString() << "\n";
    
    //printf( "%s - %s\n", typeOfMessagePrefix.c_str(), message.CString() );
    
    return true;
}
