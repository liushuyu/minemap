//
// Created by cth451 on 2020/05/15.
//

#ifndef COLORMAP_H
#define COLORMAP_H

#include <Magick++.h>
#include <memory>
#include <map>

namespace Minemap {
	struct TupleRGB {
		double r = 0.0;
		double g = 0.0;
		double b = 0.0;
		constexpr TupleRGB() = default;
		TupleRGB(double r, double g, double b) noexcept : r(r), g(g), b(b) {};

		// Comparison operators
		bool operator==(const TupleRGB &rhs) const = default;
		bool operator!=(const TupleRGB &rhs) const = default;
		friend std::strong_ordering operator<=>(const TupleRGB &lhs, const TupleRGB &rhs) = default;

		// Construct from Magick ColorRGB
		explicit TupleRGB(const Magick::ColorRGB &color) noexcept;
		// Cast to Magick ColorRGB
		explicit operator Magick::ColorRGB() const noexcept;
	};
}

namespace std {
	template<>
	struct hash<Minemap::TupleRGB> {
		/**
		 * Extended hash for the TupleRGB since Magick Quantum is very problematic.
		 * @param rgb
		 * @return
		 */
		unsigned long operator()(const Minemap::TupleRGB &rgb) const {
			auto hr = static_cast<unsigned long>(rgb.r * 256);
			auto hg = static_cast<unsigned long>(rgb.g * 256);
			auto hb = static_cast<unsigned long>(rgb.b * 256);
			return (hr << 16) + (hg << 8) + hb;
		}
	};
}

namespace Minemap {
	typedef uint8_t MapColorCode;
	/**
	 * Replaces boost::bimap. This maps minecraft map color code between RGB tuples.
	 */
	class ColorMap {
	public:
		typedef std::unordered_map<TupleRGB, MapColorCode> ReverseLookup;
		typedef TupleRGB* ForwardLookup;
	private:
		uint8_t _size;
		ReverseLookup reverseMap;
		ForwardLookup forwardMap;
	public:
		void insert(const TupleRGB& rgb, MapColorCode code);
		uint8_t size() const noexcept { return _size; };
		std::optional<TupleRGB> lookup(MapColorCode code) const noexcept;
		std::optional<MapColorCode> lookup(const TupleRGB& rgb) const noexcept;

		explicit ColorMap(uint8_t size) noexcept;
		~ColorMap() noexcept;
	};

	/**
	 * Constructs a bijective map between every color in the palette and corresponding colro code in game. This data
	 * structure allows fast lookup into both representations.
	 *
	 * @param img       Loaded Magick image
	 * @return          the said bijective map
	 */
	std::shared_ptr<ColorMap> loadColorMapFromPalette(const Magick::Image &img);
}



#endif //COLORMAP_H
