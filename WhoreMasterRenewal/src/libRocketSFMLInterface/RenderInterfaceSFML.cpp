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

#include "RenderInterfaceSFML.h"
#include "ShellFileInterface.h"

#include <Rocket/Core/Core.h>

#include <GL/gl.h>

#include <iostream>

#define GL_CLAMP_TO_EDGE 0x812F

// If built with the GL Easy Extension library we can compile geometry to VBO's
// http://www.opengl.org/sdk/libs/GLee/
#ifdef ENABLE_GLEE
#include <GL/glee.h>

class RocketSFMLRendererGeometryHandler
{
public:
    GLuint VertexID, IndexID;
    int NumVertices;
    Rocket::Core::TextureHandle Texture;
    
    RocketSFMLRendererGeometryHandler() : VertexID( 0 ), IndexID( 0 ), Texture( 0 ), NumVertices( 0 )
    {
    };
    
    ~RocketSFMLRendererGeometryHandler()
    {
        if( VertexID )
            glDeleteBuffers( 1, &VertexID );
            
        if( IndexID )
            glDeleteBuffers( 1, &IndexID );
            
        VertexID = IndexID = 0;
    };
};

#endif

struct RocketSFMLRendererVertex
{
    sf::Vector2f Position, TexCoord;
    sf::Color Color;
};

RocketSFMLRenderInterface::RocketSFMLRenderInterface()
{
}

RocketSFMLRenderInterface::~RocketSFMLRenderInterface()
{
}

void RocketSFMLRenderInterface::SetWindow( std::weak_ptr<sf::RenderWindow> windowPtr )
{
    m_WindowPtr = windowPtr;
    
    Resize();
}

std::weak_ptr<sf::RenderWindow> RocketSFMLRenderInterface::GetWindowWeakPtr()
{
    return m_WindowPtr;
}

void RocketSFMLRenderInterface::Resize()
{
    if( auto window = m_WindowPtr.lock() )
    {
        window->setActive( true );
        
        static sf::View view;
        view.setSize( static_cast<float>( window->getSize().x ), static_cast<float>( window->getSize().y ) );
        window->setView( view );
        
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, window->getSize().x, window->getSize().y, 0, -1, 1 );
        glMatrixMode( GL_MODELVIEW );
        
        glViewport( 0, 0, window->getSize().x, window->getSize().y );
    }
}

// Called by Rocket when it wants to render geometry that it does not wish to optimise.
void RocketSFMLRenderInterface::RenderGeometry( Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, const Rocket::Core::TextureHandle textureHandle, const Rocket::Core::Vector2f& translation )
{
    if( auto window = m_WindowPtr.lock() )
    {
        window->setActive();
        
        glPushMatrix();
        glTranslatef( translation.x, translation.y, 0.f );
        
        std::vector<Rocket::Core::Vector2f> positions( num_vertices );
        std::vector<Rocket::Core::Colourb> colors( num_vertices );
        std::vector<Rocket::Core::Vector2f> texCoords( num_vertices );
        
        for( int i = 0; i < num_vertices; i++ )
        {
            positions[i] = vertices[i].position;
            colors[i] = vertices[i].colour;
            texCoords[i] = vertices[i].tex_coord;
        };
        
        glEnableClientState( GL_VERTEX_ARRAY );
        
        glEnableClientState( GL_COLOR_ARRAY );
        
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        
        glVertexPointer( 2, GL_FLOAT, 0, &positions[0] );
        
        glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &colors[0] );
        
        glTexCoordPointer( 2, GL_FLOAT, 0, &texCoords[0] );
        
        glEnable( GL_BLEND );
        
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        if( textureHandle )
        {
            glEnable( GL_TEXTURE_2D );
            sf::Texture* sfTexture = reinterpret_cast<sf::Texture*>( textureHandle );
            sfTexture->bind();
        }
        else
        {
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glBindTexture( GL_TEXTURE_2D, 0 );
        };
        
        glDrawElements( GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices );
        
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        
        glDisableClientState( GL_COLOR_ARRAY );
        
        glDisableClientState( GL_VERTEX_ARRAY );
        
    //  glColor4f( 1, 1, 1, 1 );

        glDisable( GL_TEXTURE_2D );
        
        glDisable( GL_BLEND );
        
        glPopMatrix();
    }
}

// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
Rocket::Core::CompiledGeometryHandle RocketSFMLRenderInterface::CompileGeometry( Rocket::Core::Vertex* /*vertices*/,
        int /*num_vertices*/, int* /*indices*/,
        int /*num_indices*/,
        const Rocket::Core::TextureHandle /*texture*/ )
{
//#ifdef ENABLE_GLEE
//    if( auto window = m_WindowPtr.lock() )
//    {
//        window->SetActive();
//
//        if( !GLEE_VERSION_2_0 )
//          return ( Rocket::Core::CompiledGeometryHandle ) NULL;
//
//        std::vector<RocketSFMLRendererVertex> Data( num_vertices );
//
//        for( unsigned long i = 0; i < Data.size(); i++ )
//        {
//          Data[i].Position = *(sf::Vector2f*)&vertices[i].position;
//          Data[i].TexCoord = *(sf::Vector2f*)&vertices[i].tex_coord;
//          Data[i].Color = sf::Color( vertices[i].colour.red, vertices[i].colour.green,
//              vertices[i].colour.blue, vertices[i].colour.alpha );
//        };
//
//        RocketSFMLRendererGeometryHandler *geometry = new RocketSFMLRendererGeometryHandler();
//        geometry->NumVertices = num_indices;
//
//        glGenBuffers( 1, &geometry->VertexID );
//        glBindBuffer( GL_ARRAY_BUFFER, geometry->VertexID );
//        glBufferData( GL_ARRAY_BUFFER, sizeof(RocketSFMLRendererVertex) * num_vertices, &Data[0],
//          GL_STATIC_DRAW );
//
//        glGenBuffers( 1, &geometry->IndexID );
//        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, geometry->IndexID );
//        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indices, indices, GL_STATIC_DRAW );
//
//        glBindBuffer( GL_ARRAY_BUFFER, 0 );
//
//        geometry->Texture = texture;
//
//        return (Rocket::Core::CompiledGeometryHandle)geometry;
//    }
//    else
//    {
//        return 0;
//    }
//#else
    return 0;
//#endif
}

// Called by Rocket when it wants to render application-compiled geometry.
void RocketSFMLRenderInterface::RenderCompiledGeometry( Rocket::Core::CompiledGeometryHandle /*geometry*/, const Rocket::Core::Vector2f& /*translation*/ )
{
#ifdef ENABLE_GLEE
    if( auto window = m_WindowPtr.lock() )
    {
        window->SetActive();
        
        RocketSFMLRendererGeometryHandler* realGeometry = ( RocketSFMLRendererGeometryHandler* )geometry;
        
        glPushMatrix();
        glTranslatef( translation.x, translation.y, 0 );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        sf::Texture* texture = ( sf::Image* )realGeometry->Texture;
        
        if( texture )
        {
            texture->bind();
        }
        else
        {
            glBindTexture( GL_TEXTURE_2D, 0 );
        };
        
        glEnable( GL_VERTEX_ARRAY );
        
        glEnable( GL_TEXTURE_COORD_ARRAY );
        
        glEnable( GL_COLOR_ARRAY );
        
    #define BUFFER_OFFSET(x) ((char*)0 + x)
        
        glBindBuffer( GL_ARRAY_BUFFER, realGeometry->VertexID );
        
        glVertexPointer( 2, GL_FLOAT, sizeof( RocketSFMLRendererVertex ), BUFFER_OFFSET( 0 ) );
        
        glTexCoordPointer( 2, GL_FLOAT, sizeof( RocketSFMLRendererVertex ), BUFFER_OFFSET( sizeof( sf::Vector2f ) ) );
        
        glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( RocketSFMLRendererVertex ), BUFFER_OFFSET( sizeof( sf::Vector2f[2] ) ) );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, realGeometry->IndexID );
        
        glDrawElements( GL_TRIANGLES, realGeometry->NumVertices, GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
        
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        glDisable( GL_COLOR_ARRAY );
        
        glDisable( GL_TEXTURE_COORD_ARRAY );
        
        glDisable( GL_VERTEX_ARRAY );
        
        glColor4f( 1, 1, 1, 1 );
        
        glPopMatrix();
    }
#else
    ROCKET_ASSERT( false & "Not Implemented" );
#endif
}

// Called by Rocket when it wants to release application-compiled geometry.
void RocketSFMLRenderInterface::ReleaseCompiledGeometry( Rocket::Core::CompiledGeometryHandle /*geometry*/ )
{
#ifdef ENABLE_GLEE
    if( auto window = m_WindowPtr.lock() )
    {
        window->setActive();
        
        delete( RocketSFMLRendererGeometryHandler* )geometry;
    }
#else
    ROCKET_ASSERT( false & "Not Implemented" );
#endif
}

// Called by Rocket when it wants to enable or disable scissoring to clip content.
void RocketSFMLRenderInterface::EnableScissorRegion( bool enable )
{
    if( auto window = m_WindowPtr.lock() )
    {
        window->setActive();
        
        if( enable )
            glEnable( GL_SCISSOR_TEST );
        else
            glDisable( GL_SCISSOR_TEST );
    }
}

// Called by Rocket when it wants to change the scissor region.
void RocketSFMLRenderInterface::SetScissorRegion( int x, int y, int width, int height )
{
    if( auto window = m_WindowPtr.lock() )
    {
        window->setActive();
        
        glScissor( x, window->getSize().y - ( y + height ), width, height );
    }
}

// Called by Rocket when a texture is required by the library.
bool RocketSFMLRenderInterface::LoadTexture( Rocket::Core::TextureHandle& textureHandle, Rocket::Core::Vector2i& textureDimensions, const Rocket::Core::String& source )
{
    if( auto window = m_WindowPtr.lock() )
    {
        //Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_INFO, "Loading new texture: textureHandle=\"%i\" source \"%s\" textureDimensions=\"%i;%i\"", textureHandle, source.CString(), textureDimensions.x, textureDimensions.y );
        
        window->setActive();
        
        sf::Texture* sfTexture = new sf::Texture();
        
        // TODO: Texture manager based loading from file/cache
        sfTexture->loadFromFile( ( Rocket::Core::String( "Resources/" ) + source ).CString() );
        /*
        if( !sfTexture->loadFromFile( source.CString() ) )
        {
            return false;
        };
        */
        textureHandle = reinterpret_cast<Rocket::Core::TextureHandle>( sfTexture );
        textureDimensions = Rocket::Core::Vector2i( sfTexture->getSize().x, sfTexture->getSize().y );
        
        return true;
    }
    else
    {
        return false;
    }
}

// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
bool RocketSFMLRenderInterface::GenerateTexture( Rocket::Core::TextureHandle& textureHandle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& sourceDimensions )
{
    if( auto window = m_WindowPtr.lock() )
    {
        //Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_INFO, "Generating texture from existing: textureHandle=\"%i\" sourceDimensions=\"%i;%i\"", textureHandle, sourceDimensions.x, sourceDimensions.y );
        
        window->setActive();
        
        sf::Texture* texture = new sf::Texture();
        texture->create( sourceDimensions.x, sourceDimensions.y );
        texture->update( source );
        
    //  if(!image->loadFromPixels(source_dimensions.x, source_dimensions.y, source))
    //  {
    //      delete image;
    //
    //      return false;
    //  };

        textureHandle = reinterpret_cast<Rocket::Core::TextureHandle>( texture );
        
        return true;
    }
    else
    {
        return false;
    }
}

// Called by Rocket when a loaded texture is no longer required.
void RocketSFMLRenderInterface::ReleaseTexture( Rocket::Core::TextureHandle textureHandle )
{
    if( auto window = m_WindowPtr.lock() )
    {
        //Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_INFO, "Releasing texture: textureHandle=\"%i\"", textureHandle );
        
        window->setActive();
        
        delete reinterpret_cast<sf::Texture*>( textureHandle );
    }
}
