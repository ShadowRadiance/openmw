#include "constrainedfilestream.hpp"

#include <streambuf>

#include "lowlevelfile.hpp"

namespace Files
{

    class ConstrainedFileStreamBuf : public std::streambuf
    {
        // somewhat arbitrary though 64KB buffers didn't seem to improve performance any
        static const size_t sBufferSize = 4096;

        size_t mOrigin;
        size_t mSize;

        LowLevelFile mFile;

        char mBuffer[sBufferSize];

    public:
        ConstrainedFileStreamBuf(const std::string &fname, size_t start, size_t length)
        {
            mFile.open (fname.c_str ());
            mSize  = length != 0xFFFFFFFF ? length : mFile.size () - start;

            if (start != 0)
                mFile.seek(start);

            mOrigin = start;
        }

        virtual int_type underflow()
        {
            if(gptr() == egptr())
            {
                // Read in the next chunk of data, and set the read pointers on success
                size_t got = mFile.read(mBuffer, sBufferSize);
                // Failure will throw exception in LowLevelFile
                /*if(got != -1) */
                setg(&mBuffer[0], &mBuffer[0], mBuffer+got);
            }
            if(gptr() == egptr())
                return traits_type::eof();

            return traits_type::to_int_type(*gptr());
        }

        virtual pos_type seekoff(off_type offset, std::ios_base::seekdir whence, std::ios_base::openmode mode)
        {
            if((mode&std::ios_base::out) || !(mode&std::ios_base::in))
                return traits_type::eof();

            // new file position, relative to mOrigin
            size_t newPos;
            switch (whence)
            {
                case std::ios_base::beg:
                    newPos = offset;
                    break;
                case std::ios_base::cur:
                    newPos = (mFile.tell() - mOrigin) + offset;
                    break;
                case std::ios_base::end:
                    newPos = mSize + offset;
                    break;
                default:
                    return traits_type::eof();
            }
            mFile.seek(mOrigin+newPos);

            // EOF handled by exception in LowLevelFile

            // Clear read pointers so underflow() gets called on the next read attempt.
            setg(0, 0, 0);

            return newPos;
        }

        virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode)
        {
            if((mode&std::ios_base::out) || !(mode&std::ios_base::in))
                return traits_type::eof();

            if(pos >= (int)mSize)
                return traits_type::eof();
            mFile.seek(mOrigin + pos);

            // Clear read pointers so underflow() gets called on the next read attempt.
            setg(0, 0, 0);
            return pos;
        }

    };

    ConstrainedFileStream::ConstrainedFileStream(const char *filename, size_t start, size_t length)
        : std::istream(new ConstrainedFileStreamBuf(filename, start, length))
    {

    }

    ConstrainedFileStream::~ConstrainedFileStream()
    {
        delete rdbuf();
    }


    IStreamPtr openConstrainedFileStream(const char *filename,
                                                       size_t start, size_t length)
    {
        return IStreamPtr(new ConstrainedFileStream(filename, start, length));
    }
}