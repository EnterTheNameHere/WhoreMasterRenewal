#ifndef RESIZABLEIMAGE_HPP_INCLUDED_2240
#define RESIZABLEIMAGE_HPP_INCLUDED_2240
#pragma once

#include <SFGUI/Config.hpp>
#include <SFGUI/Widget.hpp>
#include <SFGUI/Misc.hpp>
#include <SFGUI/SharedPtr.hpp>
#include <SFGUI/RenderQueue.hpp>

namespace sfg
{
    /** ResizableImage
     */
    class SFGUI_API ResizableImage : public Image
    {
    public:
        typedef SharedPtr<ResizableImage> Ptr; //!< Shared pointer
        typedef SharedPtr<const ResizableImage> PtrConst; //!< Shared pointer
        
        static Ptr Create( const sf::Image& image = sf::Image() )
        {
            Ptr resizableImagePointer( new ResizableImage( image ) );
            return resizableImagePointer;
        }
        
        virtual const std::string& GetName() const
        {
            static const std::string name( "ResizableImage" );
            return name;
        }
        
        ~ResizableImage()
        {}
        
        void SetImage( const sf::Image& image )
        {
            Image::SetImage( this->m_ResizedImage );
        }
        
        const sf::Image& GetImage() const
        {
            return m_ResizedImage;
        }
        
        const void PerformResizing( const sf::Vector2u& customSize = sf::Vector2u( 0, 0 ) )
        {
            // If no size is given (0,0) , use original image...
            if( !customSize.x || !customSize.y )
            {
                Image::SetImage( m_OriginalImage );
                return;
            }
            
            // Let the flood begin
            float originalX = static_cast<float>( m_OriginalImage.getSize().x );
            float originalY = static_cast<float>( m_OriginalImage.getSize().y );
            float desiredX = static_cast<float>( customSize.x );
            float desiredY = static_cast<float>( customSize.y );
            
            // Compute the scale we will apply to image
            float scaleX = desiredX / originalX;
            float scaleY = desiredY / originalY;
            
            // Get texture of the original image
            sf::Texture tempTexture;
            tempTexture.loadFromImage( m_OriginalImage );
            
            // Sprite allows transformations to be made to it, so let's assign our texture to it
            //      (warn: sprite doesn't own the texture, keep it alive till we're done)
            sf::Sprite tempSprite;
            tempSprite.setTexture( tempTexture, true );
            
            
            if( m_KeepAspect )
            {
                // Use same scale for both sides of the sprite
                float lowerScale = std::min( scaleX, scaleY );
                
                tempSprite.scale( lowerScale, lowerScale );
                
                
                // Since the image will not be stretched out because we want to keep the aspect of the
                //  image, there will be an empty space to the right/down of the resized image. I think
                //  it's better if the empty place is distributed around the resized image on all sides,
                //  so we move the sprite to the center of the rectangle we set by size we want.
                //
                // Example
                //           ###                                ###00
                //           ###                                ###00
                //           ###                                ###00
                //           ###                                ###00
                // image 5x3 ###  resize it to 5x5, keep aspect ###00 0=empty space only on one side
                //
                //
                //                     0###0
                //                     0###0
                //                     0###0
                //                     0###0
                // so center the image 0###0 and now empty space is distributed to both sides
                
                // Move the sprite to the center of the resized image
                float offsetX = ( desiredX - ( originalX * ( lowerScale ) ) ) / 2;
                float offsetY = ( desiredY - ( originalY * ( lowerScale ) ) ) / 2;
                
                tempSprite.move( offsetX, offsetY );
            }
            else
            {
                tempSprite.scale( scaleX, scaleY );
            }
            
            // Transformations on Sprite are set, so we can pre-render the sprite on
            //  a new texture with a transparent background
            sf::RenderTexture tempRenderTexture;
            tempRenderTexture.create( customSize.x, customSize.y );
            tempRenderTexture.setSmooth( true );
            tempRenderTexture.clear( sf::Color( 255,255,255,0 ) );
            tempRenderTexture.draw( tempSprite );
            tempRenderTexture.display();
                        
            // Now then we have the new texture with our resized image ready,
            //  we can set it as a new image to the underlying base Image
            //  class, and we are done.
            Image::SetImage( tempRenderTexture.getTexture().copyToImage() );
        }
        
        const void SetKeepAspect( const bool& value = true )
        {
            this->m_KeepAspect = value;
        }
        
        const bool GetKeepAspect()
        {
            return this->m_KeepAspect;
        }
        
    protected:
        ResizableImage( const sf::Image& image = sf::Image() )
        {
            m_OriginalImage = image;
            this->SetAlignment( sf::Vector2f( .5f, .5f ) );
            this->SetImage( image );
        }
        /*
        RenderQueue* InvalidateImpl() const
        {
            WhoreMasterRenewal::Logger() << "==== ResizableImage::InvalidateImpl()\n";
            
            RenderQueue* queue = Context::Get().GetEngine().CreateImageDrawable( DynamicPointerCast<const Image>( shared_from_this() ) );
            
            m_TextureOffset = queue->GetPrimitives()[0]->GetTextures()[0]->offset;
            
            return queue;
        }*/
        
    private:
        sf::Image m_OriginalImage;
        sf::Image m_ResizedImage;
        bool m_KeepAspect = { true };
        mutable sf::Vector2f m_TextureOffset;
    };
}

#endif // RESIZABLEIMAGE_HPP_INCLUDED_2240
