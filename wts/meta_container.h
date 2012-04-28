
#ifndef WTS_META_CONTAINER_H
#define WTS_META_CONTAINER_H

#include "wts_types.h"

namespace wts
{
    typedef int DefaultSizeType;

    /** シンプルなペアクラス 
     */
    template<typename A,typename B>
    struct Pair
    {
        A a;
        B b;
        Pair(const A &a,const B &b)
            :a(a),b(b)
        {
        }
        bool operator ==(const Pair &n)const
        {
            return a==n.a&&b==n.b;
        }
        bool operator <(const Pair &n)const
        {
            if(a==n.a)
                return b<n.b;
            return a<n.a;
        }
        bool operator >(const Pair &n)const
        {
            if(a==n.a)
                return b>n.b;
            return a>n.a;
        }
    };

    /** 固定長の container 互換インターフェイス配列 
     * 確保、初期化ずみ 
     */
    template<typename T_,DefaultSizeType Size_>
    class FixedArray
    {
    public:
        typedef T_ T;

        typedef DefaultSizeType SizeType;
        static const SizeType kSize=Size_;

        WTS_CONSTEXPR SizeType Size()const
        {
            return kSize;
        }
        T* Data()
        {
            return data_;
        }
        const T* Data()const
        {
            return data_;
        }
        T& Back()
        {
            return data_[Size()-1];
        }
        const T& Back()const
        {
            return data_[Size()-1];
        }
        T& Front()
        {
            return data_[0];
        }
        const T& Front()const
        {
            return data_[0];
        }
        T* Find(const T& key)
        {
            for(SizeType i=0;i!=Size();i++)
                if(key==data_[i])
                    return data_+i;
            return NULL;
        }
        const T* Find(const T& key)const
        {
            for(SizeType i=0;i!=Size();i++)
                if(key==data_[i])
                    return data_+i;
            return NULL;
        }
        SizeType Index(const T &key)const
        {
            return static_cast<SizeType>(Find(key)-data_);
        }
        T& operator[](SizeType offset)
        {
            return data_[offset];
        }
        const T& operator[](SizeType offset)const
        {
            return data_[offset];
        }
        template<typename U>
        bool operator==(const U &cmp)const
        {
            if(!(Size()==cmp.Size()))
                return false;
            for(SizeType i=0;i!=Size();i++)
            {
                if(!(data_[i]==cmp[i]))
                    return false;
            }
            return true;
        }
        FixedArray()
        {
        }
        ~FixedArray()
        {
        }
    protected:
        T data_[kSize];
    };

    /** 可変配列アクセスの基本操作
     *
     * このままでは使えない 
     */
    template<typename T_,typename SizeType_=int>
    class BasicArray
    {
    public:
        typedef SizeType_ SizeType;
        typedef T_ T;

        SizeType Size()const
        {
            return size_;
        }
        T* Data()
        {
            return data_;
        }
        const T* Data()const
        {
            return data_;
        }
        T& Back()
        {
            return data_[size_-1];
        }
        const T& Back()const
        {
            return data_[size_-1];
        }
        T& Front()
        {
            return data_[0];
        }
        const T& Front()const
        {
            return data_[0];
        }
        bool Has(const T* pointer)const
        {
            return Data()<=pointer && pointer < Data()+Size() ;
        }
        T* Find(const T& key)
        {
            for(SizeType i=0;i!=size_;i++)
                if(key==data_[i])
                    return data_+i;
            return NULL;
        }
        const T* Find(const T& key)const
        {
            for(SizeType i=0;i!=size_;i++)
                if(key==data_[i])
                    return data_+i;
            return NULL;
        }
        SizeType Index(const T &key)const
        {
            if(T *hit=Find(key))
            {
                return static_cast<SizeType>(hit-data_);
            }
            else
            {
                return -1;
            }
        }
        T& operator[](SizeType offset)
        {
            return data_[static_cast<size_t>(offset)];
        }
        const T& operator[](SizeType offset)const
        {
            return data_[static_cast<size_t>(offset)];
        }
        template<typename T2,typename SizeType2>
        bool operator==(const BasicArray<T2,SizeType2> &cmp)const
        {
            if(!(size_==cmp.size_))
                return false;
            for(SizeType i=0;i!=size_;i++)
            {
                if(!(data_[i]==cmp.data_[i]))
                    return false;
            }
            return true;
        }
    protected:
        BasicArray(int initial_size,T* initial_pointer)
            :size_(initial_size)
            ,data_(initial_pointer)
        {
        }
        ~BasicArray()
        {
        }

        SizeType size_;
        T *data_;
    private:
        BasicArray(const BasicArray&);
        void operator=(const BasicArray&);
    };

    /** 固定長配列だけど Push や Insert できる 
     * コンテナ API 互換 
     */
    template<typename T_,typename SizeType_,SizeType_ Capacity_>
    class FixedRawArray
        :public BasicArray<T_,SizeType_>
    {
    public:
        typedef BasicArray<T_,SizeType_> Super;
        typedef typename Super::T T;
        typedef typename Super::SizeType SizeType;
        static const SizeType kCapacity=Capacity_;

        FixedRawArray()
            :Super(0,reserve_data_)
        {

        }
        FixedRawArray(const FixedRawArray &a)
            :Super(0,reserve_data_)
        {
            Insert(0,a.data_,a.size_);
        }
        FixedRawArray(const T* source,SizeType length)
            :Super(0,reserve_data_)
        {
            Insert(0,source,length);
        }
        ~FixedRawArray()
        {

        }
        SizeType Capacity()const
        {
            return kCapacity;
        }

        void Push(const T &t)
        {
            this->data_[this->size_++]=t;
        }
        T& Push()
        {
            return this->data_[this->size_++];
        }
        void Insert(SizeType offset,const T* data,SizeType length)
        {
            memmove(this->data_+offset+length,
                    this->data_+offset,
                    sizeof(T)*(this->size_-offset));
            memcpy(this->data_+offset,data,sizeof(T)*length);
            this->size_+=length;
        }
        void Erase(SizeType offset,SizeType length)
        {
            memmove(this->data_+offset,
                this->data_+offset+length,
                sizeof(T)*(this->size_-(offset+length)));
            this->size_-=length;
        }
        void Resize(SizeType new_size)
        {
            this->size_=new_size;
        }
        //void Replace(SizeType offset,SizeType length,const T *rep_data,SizeType rep_len)
        //{
        //}
        void Clear()
        {
            this->size_=0;
        }

        T reserve_data_[kCapacity];
    };


    template<typename ArrayType_>
    class FailIgnoreArray
        :public ArrayType_
    {
        typedef ArrayType_ Super;
        typedef typename Super::T T;
        typedef typename Super::SizeType SizeType;
    public:
        FailIgnoreArray()
        {

        }
        ~FailIgnoreArray()
        {

        }

        T& operator[](SizeType offset)
        {
            if(Super::Size() <= offset)
            {
                // overrun
                static T dummy_;
                return dummy_;
                //return Super::Back();
            }
            else if(offset < 0)
            {
                // underrun
                static T dummy_;
                return dummy_;
                //return Super::Front();
            }
            else
            {
                return Super::operator[](offset);
            }
        }
        const T& operator[](SizeType offset)const
        {
            return operator[](offset);
        }
        void Push(const T &t)
        {
            if(Super::Size() < Super::Capacity())
            {
                Super::Push(t);
            }
            else
            {
                // overrun
            }
        }
        T& Push()
        {
            if(Super::Size() < Super::Capacity())
            {
                return Super::Push();
            }
            else
            {
                // overrun
                static T dummy_;
                return dummy_;
            }
        }
        void Insert(SizeType offset,const T* data,SizeType length)
        {
            if(Super::Capacity() < Super::Size() + length)
            {
                // overrun
                //Insert(offset,data,length -(Super::Capacity()-Super::Size()));
            }
            else if(offset < 0)
            {
                // underrun
                //Insert(0,data-offset,length+offset);
            }
            else if(Super::Capacity() < offset+length)
            {
                //overrun
                //Insert(offset,data,Super::Capacity()-offset);
            }
            else if(Super::Size() < offset)
            {
                // flagment
                //bad
            }
            else
            {
                //good
                Super::Insert(offset,data,length);
            }
        }
        void Erase(SizeType offset,SizeType length)
        {
            if(0 <= offset &&
                offset+length <= Super::Size()
                )
            {
                Super::Erase(offset,length);
            }
            else
            {
                // err pass
            }
        }
        void Resize(SizeType new_size)
        {
            if(0 <= new_size &&
                new_size <= Super::Capacity())
                Super::Resize(new_size);
        }
        //void Replace(SizeType offset,SizeType length,const T *rep_data,SizeType rep_len)
        //{
        //}
    };
}



#endif
