/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

// mapnik
#include <mapnik/image_cast.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>

// boost
#include <boost/numeric/conversion/cast.hpp>

using boost::numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;

namespace mapnik
{

namespace detail
{

template <typename T0>
struct visitor_image_cast
{
    using dst_type = typename T0::pixel_type;
    
    T0 operator() (image_null const&) 
    {
        throw std::runtime_error("Can not cast a null image");
    }

    T0 operator() (T0 const& src)
    {
        return T0(src);
    }
    
    template <typename T1>
    T0 operator() (T1 const& src)
    {
        T0 dst(src.width(), src.height(), false);
        for (unsigned y = 0; y < dst.height(); ++y)
        {
            for (unsigned x = 0; x < dst.width(); ++x)
            {
                try
                {
                    dst(x,y) = numeric_cast<dst_type>(src(x,y));
                }
                catch(negative_overflow&)
                {
                    dst(x,y) = std::numeric_limits<dst_type>::min();
                }
                catch(positive_overflow&) 
                {
                    dst(x,y) = std::numeric_limits<dst_type>::max();
                }
            }
        }
        return T0(std::move(dst));
    }
};

template <typename T0>
struct visitor_image_cast_so
{
    using dst_type = typename T0::pixel_type;
    
    visitor_image_cast_so(double offset, double scaling)
        : offset_(offset), scaling_(scaling) {}

    T0 operator() (image_null const&) 
    {
        throw std::runtime_error("Can not cast a null image");
    }

    T0 operator() (T0 const& src)
    {
        return T0(src);
    }
    
    template <typename T1>
    T0 operator() (T1 const& src)
    {
        double src_offset = src.get_offset();
        double src_scaling = src.get_scaling();
        T0 dst(src.width(), src.height(), false);
        dst.set_scaling(scaling_);
        dst.set_offset(offset_);
        for (unsigned y = 0; y < dst.height(); ++y)
        {
            for (unsigned x = 0; x < dst.width(); ++x)
            {
                double scaled_src_val = (numeric_cast<double>(src(x,y)) * src_scaling) + src_offset;
                double dst_val = (scaled_src_val - offset_) / scaling_;
                try
                {
                    dst(x,y) = numeric_cast<dst_type>(dst_val);
                }
                catch(negative_overflow&)
                {
                    dst(x,y) = std::numeric_limits<dst_type>::min();
                }
                catch(positive_overflow&) 
                {
                    dst(x,y) = std::numeric_limits<dst_type>::max();
                }
            }
        }
        return T0(std::move(dst));
    }
  private:
    double offset_;
    double scaling_;
};

} // end detail ns

template <typename T>
MAPNIK_DECL T image_cast(image_any const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        return util::apply_visitor(detail::visitor_image_cast<T>(), data);
    }
    else
    {
        return util::apply_visitor(detail::visitor_image_cast_so<T>(offset, scaling), data);
    }
}

template <typename T>
MAPNIK_DECL T image_cast(image_rgba8 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_cast<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_cast_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template <typename T>
MAPNIK_DECL T image_cast(image_gray8 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_cast<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_cast_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template <typename T>
MAPNIK_DECL T image_cast(image_gray16 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_cast<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_cast_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template <typename T>
MAPNIK_DECL T image_cast(image_gray32f const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_cast<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_cast_so<T> visit(offset, scaling);
        return visit(data);
    }
}

MAPNIK_DECL image_any image_cast(image_any const& data, image_dtype type, double offset, double scaling)
{
    switch (type)
    {
        case image_dtype_rgba8:
            return image_any(std::move(image_cast<image_rgba8>(data, offset, scaling)));
        case image_dtype_gray8:
            return image_any(std::move(image_cast<image_gray8>(data, offset, scaling)));
        case image_dtype_gray16:
            return image_any(std::move(image_cast<image_gray16>(data, offset, scaling)));
        case image_dtype_gray32f:
            return image_any(std::move(image_cast<image_gray32f>(data, offset, scaling)));
        case image_dtype_null:
            throw std::runtime_error("Can not cast a null image");
    }
    throw std::runtime_error("Unknown image type passed");
}

} // end mapnik ns
