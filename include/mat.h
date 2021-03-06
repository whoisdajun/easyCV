//
// Created by li on 2020/10/19.
//

#ifndef EASYCV_MAT_H
#define EASYCV_MAT_H
#include <cassert>
#include <cstring>
#include <memory>
#include <functional>
#include "template_utils.h"


namespace easyCV
{

    struct BoundingBox;
    class Mat
    {
    public:
        enum class Type {
            UINT8   = 0,
            UINT16  = 2,
            INT16   = 3,
            INT32   = 4,
            FLOAT32 = 5,
            FLOAT64 = 6,
            UINT32  = 7
        };

        enum class CopyMode { Shallow, Deep };

        void* data;
        int cols; // Width
        int rows; // Height

        Mat();

        Mat(const Mat& other, CopyMode copy_mode = CopyMode::Shallow);

        Mat& operator=(const Mat& o);

        Mat& operator=(Mat&& o);

        template <typename PixelType>
        Mat& create_from_buffer(PixelType* ptr,
                                int rows,
                                int cols,
                                int channels,
                                size_t stride);

        template <typename PixelType>
        Mat& create_from_buffer(
                std::unique_ptr<PixelType, std::function<void(PixelType*)>>& ptr,
                int rows,
                int cols,
                int channels,
                size_t stride);

        template <typename PixelType>
        Mat& create(int rows, int cols, int channels);

        template <typename PixelType>
        Mat& operator<<(const std::initializer_list<PixelType>& fill_data)
        {
            const int num_channels = channels();

            assert(fill_data.size() ==
                   static_cast<size_t>(rows * cols * num_channels));

            auto src_it = fill_data.begin();
            Iterator<PixelType> dst_it(*this);

            for (int y = 0; y < rows; ++y) {
                for (int x = 0; x < cols; ++x) {
                    for (int c = 0; c < num_channels; ++c) {
                        dst_it(y, x, c) = *src_it;
                        ++src_it;
                    }
                }
            }

            return *this;
        }

        Mat& operator<<(const Mat& from)
        {
            assert(!empty());
            assert(!from.empty());
            assert(from.rows == rows);
            assert(from.cols == cols);
            assert(from.channels() == channels());
            assert(from.type() == type());

            memcpy(data,
                   from.data,
                   from.rows * from.cols * from.channels() * from.sizeof_type());

            return *this;
        }

        template <typename PixelType>
        Mat& fill(typename dependent_type<PixelType>::type value)
        {
            assert(get_type_enum<PixelType>() == type());

            const int num_channels = channels();
            Iterator<PixelType> dst_it(*this);

            for (int y = 0; y < rows; ++y) {
                for (int x = 0; x < cols; ++x) {
                    for (int c = 0; c < num_channels; ++c) {
                        dst_it(y, x, c) = value;
                    }
                }
            }

            return *this;
        }

        template <typename InputType, typename OutputType>
        Mat convert() const
        {
            const int num_channels = channels();

            Mat output;
            output.create<OutputType>(rows, cols, num_channels);

            ConstIterator<InputType> src_it(*this);
            Iterator<OutputType> dst_it(output);

            for (int y = 0; y < rows; ++y) {
                for (int x = 0; x < cols; ++x) {
                    for (int c = 0; c < num_channels; ++c) {
                        dst_it(y, x, c) = static_cast<OutputType>(src_it(y, x, c));
                    }
                }
            }

            return output;
        }

        template <typename IteratorType>
        void for_each(const std::function<void(IteratorType&, int, int, int)>& cbk)
        {
            IteratorType it(*this);
            int num_channels = channels();

            for (int y = 0; y < rows; ++y) {
                for (int x = 0; x < cols; ++x) {
                    for (int c = 0; c < num_channels; ++c) {
                        cbk(it, y, x, c);
                    }
                }
            }
        }

        Type type() const;

        bool empty() const;

        void release();

        int channels() const
        {
            return static_cast<int>(step.buf[1]);
        }

        size_t row_stride() const
        {
            return step.buf[0];
        }

        template <typename PixelType>
        static Type get_type_enum()
        {
            if (std::is_same<PixelType, uint8_t>::value) {
                return Type::UINT8;
            } else if (std::is_same<PixelType, uint16_t>::value) {
                return Type::UINT16;
            } else if (std::is_same<PixelType, int16_t>::value) {
                return Type::INT16;
            } else if (std::is_same<PixelType, int32_t>::value) {
                return Type::INT32;
            } else if (std::is_same<PixelType, float>::value) {
                return Type::FLOAT32;
            } else if (std::is_same<PixelType, double>::value) {
                return Type::FLOAT64;
            } else if (std::is_same<PixelType, uint32_t>::value) {
                return Type::UINT32;
            }
        }

        size_t sizeof_type() const
        {
            // TODO assert valid matrix

            size_t result;

            switch (type()) {
                case Type::UINT8:
                    result = sizeof(uint8_t);
                    break;
                case Type::UINT16:
                    result = sizeof(uint16_t);
                    break;
                case Type::INT16:
                    result = sizeof(int16_t);
                    break;
                case Type::INT32:
                    result = sizeof(int32_t);
                    break;
                case Type::FLOAT32:
                    result = sizeof(float);
                    break;
                case Type::FLOAT64:
                    result = sizeof(double);
                    break;
                case Type::UINT32:
                    result = sizeof(uint32_t);
                    break;
                default:
                    result = 1;

                    assert(!"Invalid pixel type");
            }

            return result;
        }

        bool is_mask_of(const Mat& image) const
        {
            return rows == image.rows && cols == image.cols &&
                   type() == Type::UINT8;
        }

        template <typename T>
        struct Iterator
        {
            using PixelType = T;

            Iterator(Mat& m_)
                    : m(m_)
            {
                assert(get_type_enum<PixelType>() == m.type());
            }

            Iterator<T>& operator=(Iterator<T>& o);

            T& operator()(int row, int col, int chan)
            {
                assert(row >= 0);
                assert(row < m.rows);
                assert(col >= 0);
                assert(col < m.cols);

                return (static_cast<T*>(
                        m.data))[row * m.step.buf[0] + col * m.step.buf[1] + chan];
            }

            Mat& m;
        };

        template <typename T>
        struct ConstIterator
        {
            using PixelType = T;

            ConstIterator(const Mat& m_)
                    : m(m_)
            {
                assert(get_type_enum<PixelType>() == m.type());
            }

            ConstIterator(const ConstIterator& cv_it)
                    : m(cv_it.m)
            {
            }

            ConstIterator(const Iterator<T>& cv_it)
                    : m(cv_it.m)
            {
            }

            ConstIterator<T>& operator=(ConstIterator<T>& o);

            const T& operator()(int row, int col, int chan) const
            {

                assert(row >= 0);
                assert(row < m.rows);
                assert(col >= 0);
                assert(col < m.cols);
                assert(chan >= 0);
                assert(chan < m.channels());

                return (static_cast<T*>(
                        m.data))[row * m.step.buf[0] + col * m.step.buf[1] + chan];
            }

            bool is_mask_of(const Mat& image) const
            {
                return m.is_mask_of(image);
            }

            const Mat& m;
        };

        /// Friends
        template <typename T>
        friend Mat image_crop(const Mat& image, const BoundingBox& crop_bb);

    private:
        struct
        {
            size_t buf[3]; // buf[0] = width of the containing buffer*channels;
            // buf[1] = channels
            // buf[2] = sizeof(PixelType)
        } step;

        std::shared_ptr<void> data_mgr_;

        int flags_; // OpenCV-compatible flags

        template <typename T>
        void compute_flags(int channels);
    };

    constexpr Mat::Type SatTypeEnum = Mat::Type::INT32;
    using SatType                   = int32_t;
}
#endif //EASYCV_MAT_H
