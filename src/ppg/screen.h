#include "widget.h"

namespace ppg {
/**
 * @class PpgScreen
 * @ingroup Ppg
 * @brief The virtual DOS screen
 */
class Screen : public Widget {
		DISABLE_COPY(Screen)
	private:
		/**
		 * @brief Draw an 8x8 char
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Char to drawBgColor
		 * @param[in] foreground Foreground color
		 * @param[in] background Background color
		 * @param[in] opaque Set to @c false to draw a transparent char
		 */
		void drawChar8(int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque = true) throw();
		/**
		 * @copydoc PpgScreen::drawChar8
		 * @brief Draw an 8x16 char
		 */
		void drawChar16(int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque = true) throw();
		virtual void drawThis() throw(Exception);
		int m_cursorX;
		int m_cursorY;
	public:
		/**
		 * @brief Create a new virtual DOS screen
		 * @param[in] w Width in characters
		 * @param[in] h Height in characters
		 * @param[in] title Title of the screen
		 */
		Screen(const int w, const int h, const std::string& title) throw(Exception);
		virtual ~Screen() throw();
		/**
		 * @brief Clear the screen
		 * @param[in] c Character to overwrite the screen with
		 * @param[in] foreground Foreground color
		 * @param[in] background Background color
		 */
		void clear(uint8_t c, uint8_t foreground, uint8_t background) throw();
		virtual void drawChar(int x, int y, char c) throw();
		virtual void drawFgColor(int x, int y, uint8_t c) throw();
		virtual void drawBgColor(int x, int y, uint8_t c) throw();
		/**
		 * @brief Clear the pixel overlay (make it fully transparent)
		 */
		void clearOverlay();
		void drawPixel(int x, int y, uint8_t color);
		void setCursor(int x, int y) { m_cursorX=x; m_cursorY=y; }
};
} // namespace ppg