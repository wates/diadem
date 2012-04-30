
#ifndef WTS_CONTAINER_H_
#define WTS_CONTAINER_H_

#include "allocator.h"

#include <string.h>

namespace wts
{

    template<typename T_,typename SizeType_=int>
    class DynamicRawArray
        :public BasicArray<T_,SizeType_>
    {
    public:
        typedef BasicArray<T_,SizeType_> Super;
        typedef typename Super::T T;
        typedef typename Super::SizeType SizeType;

        DynamicRawArray()
            :Super(0,NULL)
            ,reserve_(0)
        {
        }
        DynamicRawArray(const DynamicRawArray &copy)
            :reserve_((copy.Size()+15)&0xf)
            ,Super(copy.Size(),reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(this->reserve_*sizeof(T))))
        {
            memcpy(this->data_,copy.Data(),this->size_);
        }
        void operator =(const DynamicRawArray &copy)
        {
            Clear();
            Insert(0,copy.Data(),copy.Size());
        }
        ~DynamicRawArray()
        {
            if(this->data_)
                AllocatorSelector<T>::Free(this->data_);
        }
        void Insert(SizeType offset,const T* data,SizeType length)
        {
            if(this->size_+length>this->reserve_)
            {
                SizeType new_size=this->size_+length;
                SizeType new_reserve=(new_size*2+15);
                new_reserve-=new_reserve&0xf;

                T *new_data=reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(new_reserve*sizeof(T)));
                if(offset>0)
                    memcpy(new_data,this->data_,offset*sizeof(T));
                if(offset!=this->size_)
                    memcpy(new_data+offset+length,this->data_+offset,(this->size_-offset)*sizeof(T));
                this->reserve_=new_reserve;
                if(this->data_)
                    AllocatorSelector<T>::Free(this->data_);
                this->data_=new_data;
            }
            else
            {
                if(offset!=this->size_)
                    memmove(this->data_+offset+length,this->data_+offset,(this->size_-offset)*sizeof(T));
            }
            if(data)
                memcpy(this->data_+offset,data,length*sizeof(T));
            this->size_+=length;
        }
        void Erase(SizeType offset,SizeType length)
        {
            if(offset+length!=this->size_)
            {
                memmove(this->data_+offset,this->data_+offset+length,(this->size_-offset-length)*sizeof(T));
            }
            this->size_-=length;
        }
        void Resize(SizeType new_size)
        {
            if(new_size>this->reserve_)
            {
                SizeType new_reserve=(new_size*2+15);
                new_reserve-=new_reserve&0xf;
                T *new_data=reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(new_reserve*sizeof(T)));
                memcpy(new_data,this->data_,this->size_*sizeof(T));
                if(this->data_)
                    AllocatorSelector<T>::Free(this->data_);
                this->data_=new_data;
                reserve_=new_reserve;
            }
            this->size_=new_size;
        }
        void Replace(SizeType offset,SizeType length,const T *rep_data,SizeType rep_len)
        {
            if(length>rep_len)
                Erase(offset+rep_len,length-rep_len);
            else if(length<rep_len)
                Insert(offset+length,NULL,rep_len-length);
            memcpy(this->data_+offset,rep_data,rep_len*sizeof(T));
        }
        void Clear()
        {
            this->size_=0;
        }
        T& Push()
        {
            Resize(this->size_+1);
            return this->data_[this->size_-1];
        }
        void Push(const T& copy)
        {
            Push()=copy;
        }
        void Pop()
        {
            Resize(this->size_-1);
        }
    protected:
        SizeType reserve_;
    private:
    };


    template<typename T_,typename SizeType_=int,int Upkeep_=16>
    class RawArray
        :public DynamicRawArray<T_,SizeType_>
    {
    public:
        typedef DynamicRawArray<T_,SizeType_> Super;
        typedef typename Super::SizeType SizeType;
        typedef typename Super::T T;
        static const SizeType Upkeep=Upkeep_;

        RawArray()
            :Super()
            ,is_local_(true)
        {
            this->reserve_=Upkeep;
            this->data_=reinterpret_cast<T*>(this->local_);
        }
        RawArray(const RawArray &copy)
            :Super()
            ,is_local_(true)
        {
            this->reserve_=Upkeep;
            this->data_=reinterpret_cast<T*>(this->local_);
            Insert(0,copy.Data(),copy.Size());
        }
        ~RawArray()
        {
            if(!is_local_)
                AllocatorSelector<T>::Free(this->data_);
            this->data_=NULL;
        }
        RawArray& operator=(const RawArray &copy)
        {
            Clear();
            Insert(0,copy.Data(),copy.Size());
            return *this;
        }
        void Clear()
        {
            if(!is_local_)
            {
                Super::Clear();
                AllocatorSelector<T>::Free(this->data_);
                this->is_local_=true;
            }
            else
            {
                this->size_=0;
            }
            this->reserve_=Upkeep;
            this->data_=reinterpret_cast<T*>(this->local_);
        }
        void Insert(SizeType offset,const T* data,SizeType length)
        {
            if(this->is_local_)
            {
                if(length+this->size_>this->reserve_)
                {
                    SizeType new_size=this->size_+length;
                    SizeType new_reserve=(new_size*2+15);
                    new_reserve-=new_reserve&0xf;

                    T *new_data=reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(new_reserve*sizeof(T)));
                    if(offset>0)
                        memcpy(new_data,this->data_,offset*sizeof(T));
                    if(offset!=this->size_)
                        memcpy(new_data+offset+length,this->data_+offset,(this->size_-offset)*sizeof(T));
                    this->reserve_=new_reserve;
                    this->data_=new_data;
                    this->is_local_=false;
                }
                else
                {
                    if(offset!=this->size_)
                        memmove(this->data_+offset+length,this->data_+offset,(this->size_-offset)*sizeof(T));
                }
                if(data)
                    memcpy(this->data_+offset,data,length*sizeof(T));
                this->size_+=length;
            }
            else
            {
                Super::Insert(offset,data,length);
            }
        }
        void Resize(SizeType new_size)
        {
            if(this->is_local_)
            {
                if(Upkeep<new_size)
                {
                    SizeType new_reserve=(new_size*2+15);
                    new_reserve-=new_reserve&0xf;
                    T *new_data=reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(new_reserve*sizeof(T)));
                    memcpy(new_data,this->data_,this->size_*sizeof(T));
                    this->data_=new_data;
                    this->is_local_=false;
                }
                this->size_=new_size;
            }
            else
            {
                if(new_size<Upkeep)
                {
                    AllocatorSelector<T>::Free(this->data_);
                    this->data_=reinterpret_cast<T*>(this->local_);
                    this->is_local_=true;
                }
                else
                {
                    Super::Resize(new_size);
                }
            }
        }
        void Append(const T *data,SizeType length)
        {
            Insert(this->size_,data,length);
        }
        T& Push()
        {
            Resize(this->size_+1);
            return Super::Back();
        }
        void Push(const T &t)
        {
            Push()=t;
        }
    private:
        char local_[sizeof(T)*Upkeep];
        bool is_local_;
    };

    template<typename T_,typename SizeType_=int>
    class DynamicArray
        :public BasicArray<T_,SizeType_>
    {
    public:
        typedef BasicArray<T_,SizeType_> Super;
        typedef typename Super::T T;
        typedef typename Super::SizeType SizeType;

        DynamicArray()
            :Super(0,NULL)
        {
            this->reserve_=0;
        }
        DynamicArray(const DynamicArray &copy)
            :Super(copy.size_,reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(copy.size_*sizeof(T))))
        {
            this->reserve_=(this->size_+15);
            this->reserve_-=this->reserve_&15;

            for(SizeType i=0;i<this->size_;++i)
                new(this->data_+i)T(copy[i]);
        }
        DynamicArray& operator=(const DynamicArray &copy)
        {
            Clear();
            Insert(0,copy.Data(),copy.Size());
            return *this;
        }
        ~DynamicArray()
        {
            if(this->data_)
            {
                for(SizeType i=0;i<this->size_;++i)
                    this->data_[i].~T();
                AllocatorSelector<T>::Free(this->data_);
            }
        }
        void Allocate(SizeType offset,SizeType length)
        {
            if(this->size_+length>this->reserve_)
            {
                SizeType new_size=this->size_+length;
                SizeType new_reserve=(new_size*2+15);
                new_reserve-=new_reserve&0xf;

                T *new_data=reinterpret_cast<T*>(AllocatorSelector<T>::Alloc(new_reserve*sizeof(T)));
                for(SizeType i=0;i<offset;++i)
                {
                    new(new_data+i)T(this->data_[i]);
                    this->data_[i].~T();
                }
                for(SizeType i=offset;i<this->size_;++i)
                {
                    new(new_data+length+i)T(this->data_[i]);
                    this->data_[i].~T();
                }
                this->reserve_=new_reserve;
                if(this->data_)
                    AllocatorSelector<T>::Free(this->data_);
                this->data_=new_data;
            }
            else
            {
                if(this->size_<offset+length)
                {
                    for(SizeType i=offset+length;i<this->size_+length;++i)
                    {
                        new(this->data_+i)T(this->data_[i-length]);
                        this->data_[i-length].~T();
                    }
                }
                else
                {
                    for(SizeType i=this->size_;i<this->size_+length;++i)
                        new(this->data_+i)T(this->data_[i-length]);
                    for(SizeType i=this->size_-length-1;i!=offset-1;--i)
                        this->data_[i+length]=this->data_[i];
                    for(SizeType i=offset;i<offset+length;++i)
                        this->data_[i].~T();
                }
            }
            this->size_+=length;
        }
        void Insert(SizeType offset,const T* data,SizeType length)
        {
            if(length<=0)
                return;
            Allocate(offset,length);
            if(data)
            {
                for(SizeType i=0;i<length;i++)
                    new(this->data_+offset+i)T(data[i]);
            }
            else
            {
                for(SizeType i=0;i<length;i++)
                    new(this->data_+offset+i)T();
            }
        }
        T& Push()
        {
            Allocate(this->size_,1);
            new(&Super::Back())T();
            return Super::Back();
        }
        void Push(const T &copy)
        {
            Allocate(this->size_,1);
            new(&Super::Back())T(copy);
        }
        void Pop()
        {
            Super::Back().~T();
            --this->size_;
        }
        void Erase(SizeType offset,SizeType length)
        {
            if(length<=0)
                return;
            for(SizeType i=offset;i<this->size_;++i)
            {
                if(i+length<this->size_)
                {
                    this->data_[i]=this->data_[i+length];
                }
                else
                {
                    this->data_[i].~T();
                }
            }
            this->size_-=length;
        }
        void Resize(SizeType new_size)
        {
            if(this->size_<new_size)
            {
                Insert(this->size_,NULL,new_size-this->size_);
            }
            else if(this->size_>new_size)
            {
                Erase(new_size,this->size_-new_size);
            }
        }
        void Clear()
        {
            Erase(0,this->size_);
        }
    protected:
        SizeType reserve_;
    };
#define Array DynamicArray

    template<typename ArrayType_>
    class BasicString
    {
    public:
        typedef ArrayType_ ArrayType;
        typedef typename ArrayType::T Char;
        typedef typename ArrayType::SizeType SizeType;
    public:
        static SizeType strlen(const Char *str)
        {
            SizeType n=0;
            while(str[n])
                ++n;
            return n;
        }
        ArrayType& Content()
        {
            return content_;
        }
        const ArrayType& Content()const
        {
            return content_;
        }
        BasicString()
        {
            content_.Push(0);
        }
        BasicString(const Char *src)
        {
            content_.Insert(0,src,strlen(src)+1);
        }
        BasicString& operator=(const Char *source)
        {
            content_.Clear();
            content_.Insert(0,source,strlen(source)+1);
            return *this;
        }
        BasicString(const BasicString &copy)
            :content_(copy.Content())
        {
        }
        BasicString& operator=(const BasicString &copy)
        {
            content_=copy.Content();
            return *this;
        }
        void Replace(SizeType offset,SizeType length,const Char *rep)
        {
            SizeType rep_len=strlen(rep);
            content_.Replace(offset,length,rep,rep_len);
        }
        BasicString& Assign(const Char *source,SizeType len)
        {
            content_.Clear();
            content_.Insert(0,source,len);
            if(0!=content_.Back())
                content_.Push(0);
            return *this;
        }
        SizeType Size()const
        {
            return content_.Size()-1;
        }
        Char* Data()
        {
            return content_.Data();
        }
        const Char* Data()const
        {
            return content_.Data();
        }
        SizeType Find(Char c,SizeType offset=0)const
        {
            for(;offset<content_.Size();offset++)
                if(content_[offset]==c)
                    return offset;
            return -1;
        }
        SizeType Find(const Char *str,SizeType offset=0)const
        {
            SizeType len=strlen(str);
            for(;offset<=content_.Size()-len;offset++)
                if(0==memcmp(content_.Data()+offset,str,sizeof(Char)*len))
                    return offset;
            return -1;
        }
        void Clear()
        {
            content_.Clear();
            content_.Push(0);
        }
        void Pop()
        {
            content_.Pop();
            content_.Back()=0;
        }
        BasicString& operator+=(const Char *append)
        {
            content_.Pop();
            content_.Insert(content_.Size(),append,strlen(append)+1);
            return *this;
        }
        void Append(Char c)
        {
            content_.Back()=c;
            content_.Push(0);
        }
        void Append(const BasicString &append)
        {
            content_.Pop();
            content_.Append(append.Data(),append.Size());
            content_.Push(0);
        }
        void Erase(SizeType offset,SizeType length)
        {
            content_.Erase(offset,length);
        }
        bool operator<(const BasicString &cmp)const
        {
            return strcmp(content_.Data(),cmp.Content().Data())<0;
        }
        bool operator>(const BasicString &cmp)const
        {
            return strcmp(content_.Data(),cmp.Content().Data())>0;
        }
        bool operator==(const BasicString &cmp)const
        {
            return content_==cmp.Content();
        }
        Char& operator[](SizeType n)
        {
            return content_[n];
        }
        const Char& operator[](SizeType n)const
        {
            return content_[n];
        }
    private:
        //template<typename T>void operator=(const T &copy);
        ArrayType content_;
    };

    typedef BasicString<RawArray<char> > String;
    typedef BasicString<RawArray<wchar_t> > Wstring;

    template<typename IN,typename OUT>
    void StrToStr(const IN &in,OUT *out)
    {
        out->Clear();
        typename OUT::SizeType len=IN::strlen(in.Data());
        for(typename OUT::SizeType i=0;i<len;i++)
            out->Append(static_cast<typename OUT::Char>(in[i]));
    }

    struct CompareNormal
    {
        template<typename A,typename B>
        inline bool Greater(const A&a,const B&b)const
        {
            return a<b;
        }
        template<typename A,typename B>
        inline bool Less(const A&a,const B&b)const
        {
            return a>b;
        }
        template<typename A,typename B>
        inline bool Equal(const A&a,const B&b)const
        {
            return a==b;
        }

    };

    template<typename T>
    struct TreeNode
    {
        T* left;
        T* right;
    };

    template<typename T_>
    class TreeIterator
    {
    public:
        typedef T_ T;

        template<typename Tree>
        TreeIterator(Tree &tree)
        {
            if(0==tree.Root())
                return;
            route_.Push(tree.Root());
            while(route_.Back()->left)
                route_.Push(route_.Back()->left);
        }
        operator bool()const
        {
            return route_.Size()!=0;
        }
        void operator++()
        {
            T *b=route_.Back();
            route_.Pop();
            if(b->right)
            {
                route_.Push(b->right);
                while(route_.Back()->left)
                    route_.Push(route_.Back()->left);
            }
        }
        typename T::T& operator*()
        {
            return route_.Back()->content_;
        }
        typename T::T* operator->()
        {
            return &(route_.Back()->content_);
        }
    private:
        RawArray<T*,int,32> route_;

    };

    template<typename T_,typename CmpFunc_,typename Node_>
    class Tree
    {
    public:
        typedef Node_ Node;
        typedef TreeIterator<Node> Iterator;
        typedef T_ T;


        Tree()
            :root_(0)
        {
        }
        ~Tree()
        {
            Clear();
        }
        void Clear()
        {
            Array<Node*> stack;
            stack.Push(root_);
            while(stack.Size())
            {
                Node *n=stack.Back();
                stack.Pop();
                if(0==n)
                    continue;
                if(n->left)
                    stack.Push(n->left);
                if(n->right)
                    stack.Push(n->right);
                delete n;
            }
            root_=0;
        }
        Node* Root()const
        {
            return root_;
        }
        T& Front()const
        {
            Node *n=root_;
            while(n->left)
                n=n->left;
            return n->content_;
        }
        T& Back()const
        {
            Node *n=root_;
            while(n->right)
                n=n->right;
            return n->content_;
        }
        template<typename F>
        void Foreach(F& func,Node *from)
        {
            if(from->left)
                Foreach(func,from->left);
            func(from->content_);
            if(from->right)
                Foreach(func,from->right);
        }
        template<typename F>
        void Foreach(F& func)
        {
            if(root_)
                Foreach(func,root_);
        }
        template<typename F>
        void Foreach(F& func,Node *from)const
        {
            if(from->left)
                Foreach(func,from->left);
            func(from->content_);
            if(from->right)
                Foreach(func,from->right);
        }
        template<typename F>
        void Foreach(F& func)const
        {
            if(root_)
                Foreach(func,root_);
        }
        template<typename N>
        T* Find(const N &key)const
        {
            Node *n=root_;
            while(n)
            {
                if(cmp_.Greater(n->content_,key))
                    n=n->right;
                else if(cmp_.Less(n->content_,key))
                    n=n->left;
                else
                    return &n->content_;
            }
            return 0;
        }
    protected:
        Node *root_;
        CmpFunc_ cmp_;
    };

    template<typename T_>
    class BinaryNode
        :public TreeNode<BinaryNode<T_> >
    {
    public:
        typedef T_ T;
        template<typename N>
        explicit BinaryNode(const N&n)
            :content_(n){}
        T content_;
    };

    template<typename T_,typename CmpFunc_=CompareNormal>
    class BinaryTree
        :public Tree<T_,CmpFunc_,BinaryNode<T_> >
    {
    public:
        typedef Tree<T_,CmpFunc_,BinaryNode<T_> > Super;
        typedef typename Super::T T;
        typedef typename Super::Node Node;
        typedef typename Super::Iterator Iterator;

        template<typename N>
        T& Insert(const N &key)
        {
            Node *p=0;
            Node **n=&this->root_;
            while(*n)
            {
                p=*n;
                if(this->cmp_.Greater(p->content_,key))
                    n=&(p->right);
                else if(this->cmp_.Less(p->content_,key))
                    n=&(p->left);
                else
                {
                    p->content_=key;
                    return p->content_;
                }
            }
            (*n)=new Node(key);
            (*n)->right=0;
            (*n)->left=0;
            return (*n)->content_;
        }
        template<typename N>
        void Erase(const N &key)
        {
            Node *p;
            Node **n=&this->root_;
            while(*n)
            {
                if(this->cmp_.Greater((*n)->content_,key))
                    n=&((*n)->right);
                else if(this->cmp_.Less((*n)->content_,key))
                    n=&((*n)->left);
                else
                    break;
            }
            p=*n;
            if(p->left&&p->right)
            {
                Node *s;
                Node **t=&p->right;
                while((*t)->left)
                    t=&(*t)->left;
                s=*t;
                *t=s->right;
                s->left=p->left;
                s->right=p->right;
                *n=s;
                delete p;
            }
            else if(p->left)
            {
                *n=p->left;
                delete p;
            }
            else if(p->right)
            {
                *n=p->right;
                delete p;
            }
            else
            {
                *n=0;
                delete p;
            }
        }
    };

    template<typename T_>
    class AvlNode
        :public TreeNode<AvlNode<T_> >
    {
    public:
        typedef T_ T;
        template<typename Any>
        explicit AvlNode(const Any &content)
            :content_(content)
        {

        }
        template<typename Any>
        void operator=(const Any &content)
        {
            content_=content;
        }
        T content_;
        int balance_;
    };

    template<typename T_,typename CmpFunc_=CompareNormal>
    class AvlTree
        :public Tree<T_,CmpFunc_,AvlNode<T_> >
    {
    public:
        typedef Tree<T_,CmpFunc_,AvlNode<T_> > Super;
        typedef typename Super::T T;
        typedef typename Super::Node Node;
        typedef typename Super::Iterator Iterator;

    private:
        enum
        {
            LEFT,
            EQUAL,
            RIGHT
        };
    private:
        template<typename N>
        T& AddNode(Node *&parent,const N& value,bool &grow)
        {
            if(0==parent)
            {
                parent=new Node(value);
                parent->balance_=EQUAL;
                parent->left=0;
                parent->right=0;
                grow=true;
                ++size_;
                return parent->content_;
            }

            if(this->cmp_.Less(parent->content_,value))
            {
                T &c=AddNode(parent->left,value,grow);
                if(!grow)
                    return c;

                if(RIGHT==parent->balance_)
                {
                    parent->balance_=EQUAL;
                    grow=false;
                }
                else if(EQUAL==parent->balance_)
                {
                    parent->balance_=LEFT;
                }
                else
                {
                    AdjustLeftGrow(parent);
                    grow=false;
                }
                return c;
            }
            else
            {
                if(this->cmp_.Greater(parent->content_,value))
                {
                    T &c=AddNode(parent->right,value,grow);
                    if(!grow)
                        return c;
                    if(LEFT==parent->balance_)
                    {
                        parent->balance_=EQUAL;
                        grow=false;
                    }
                    else if(EQUAL==parent->balance_)
                    {
                        parent->balance_=RIGHT;
                    }
                    else
                    {
                        AdjustRightGrow(parent);
                        grow=false;
                    }
                    return c;
                }
                else
                {
                    // 同じ場合.
                    parent->content_=value;
                    return parent->content_;
                }
            }
        }
        void AdjustLeftGrow(Node *&parent)
        {
            Node *lc=parent->left;
            Node *gc;
            if(LEFT==lc->balance_)
            {
                //左側の左側でバランスが崩れたので、右回転する.
                parent->left=lc->right;
                lc->right=parent;
                parent->balance_=EQUAL;
                parent=lc;
            }
            else
            {
                //左側の右側でバランスが崩れたので、左－右回転する.
                gc=lc->right;
                lc->right=gc->left;
                gc->left=lc;
                parent->left=gc->right;
                gc->right = parent;
                if (LEFT==gc->balance_)
                    parent->balance_ = RIGHT;
                else
                    parent->balance_ = EQUAL;
                if (RIGHT==gc->balance_)
                    lc->balance_ = LEFT;
                else
                    lc->balance_ = EQUAL;
                parent = gc;
            }
            parent->balance_=EQUAL;
        }
        void AdjustRightGrow(Node *&parent)
        {
            Node *rc=parent->right;
            Node *gc;
            if (RIGHT==rc->balance_)
            {
                //右側の右側でバランスが崩れたので、左回転する.
                parent->right = rc->left;
                rc->left = parent;
                parent->balance_ = EQUAL;
                parent = rc;
            }
            else
            {
                //右側の左側でバランスが崩れたので、右－左回転する.
                gc = rc->left;
                rc->left = gc->right;
                gc->right = rc;
                parent->right = gc->left;
                gc->left = parent;
                if (RIGHT==gc->balance_)
                    parent->balance_ = LEFT;
                else
                    parent->balance_ = EQUAL;
                if (LEFT==gc->balance_)
                    rc->balance_ = RIGHT;
                else
                    rc->balance_ = EQUAL;
                parent = gc;
            }
            parent->balance_ = EQUAL;
        }
        bool AdjustLeftCutdown(Node *&node)
        {
            if(LEFT==node->balance_) 
            {
                node->balance_ = EQUAL;
                return true;
            }
            else if(EQUAL==node->balance_) 
            {
                node->balance_ = RIGHT;
                return false;
            }
            else
            {
                Node *rc = node->right;
                int cb = rc->balance_;
                if(LEFT != cb) 
                {
                    node->right = rc->left;
                    rc->left = node;
                    if (EQUAL==cb) 
                    {
                        node->balance_ = RIGHT;
                        rc->balance_ = LEFT;
                        node = rc;
                        return false;
                    }
                    else
                    {
                        node->balance_ = EQUAL;
                        rc->balance_ = EQUAL;
                        node = rc;
                        return true;
                    }
                }
                else
                {
                    Node *gc = rc->left;
                    int gb = gc->balance_;
                    rc->left = gc->right;
                    gc->right = rc;
                    node->right = gc->left;
                    gc->left = node;

                    if (RIGHT==gb) 
                        node->balance_ = LEFT;
                    else
                        node->balance_ = EQUAL;

                    if (LEFT==gb) 
                        rc->balance_ = RIGHT;
                    else
                        rc->balance_ = EQUAL;

                    node = gc;
                    gc->balance_ = EQUAL;

                    return true;
                }
            }
        }
        bool AdjustRightCutdown(Node *&node)
        {
            if (RIGHT==node->balance_) 
            {
                node->balance_ = EQUAL;
                return true;
            }
            else if (EQUAL==node->balance_) 
            {
                node->balance_ = LEFT;
                return false;
            }
            else
            {
                Node *lc=node->left;
                int cb=lc->balance_;
                if (RIGHT!=cb) 
                {
                    node->left = lc->right;
                    lc->right = node;
                    if (EQUAL==cb) 
                    {
                        node->balance_ = LEFT;
                        lc->balance_ = RIGHT;
                        node = lc;
                        return false;
                    }
                    else
                    {
                        node->balance_ = EQUAL;
                        lc->balance_ = EQUAL;
                        node = lc;
                        return true;
                    }
                }
                else
                {
                    Node *gc=lc->right;
                    int gb=gc->balance_;
                    lc->right = gc->left;
                    gc->left = lc;
                    node->left = gc->right;
                    gc->right = node;

                    if (LEFT==gb) 
                        node->balance_ = RIGHT;
                    else
                        node->balance_ = EQUAL;

                    if (RIGHT==gb) 
                        lc->balance_ = LEFT;
                    else
                        lc->balance_ = EQUAL;

                    node = gc;
                    gc->balance_ = EQUAL;

                    return true;
                }
            }
        }
        void ReplaceRightMost(Node *&target,Node *&replace,bool &cutdown)
        {
            if (0==replace->right) 
            {
                Node *on;
                on = replace;
                replace = replace->left;
                on->left  = target->left;
                on->right = target->right;
                on->balance_    = target->balance_;
                target = on;
                cutdown = true;
            }
            else
            {
                Node *rr=replace->right;
                ReplaceRightMost(target, rr, cutdown);
                replace->right=rr;
                if (cutdown)
                    cutdown = AdjustRightCutdown(replace);
            }
        }
        template<typename N>
        void DeleteFromNode(Node *&node,const N &content,bool &cutdown)
        {
            if (!node)
            {
                cutdown = false;
                return;
            }

            if (this->cmp_.Less(node->content_,content))
            {
                DeleteFromNode(node->left, content, cutdown);
                if(cutdown)
                    cutdown = AdjustLeftCutdown(node);
            }
            else if (this->cmp_.Greater(node->content_,content))
            {
                DeleteFromNode(node->right, content, cutdown);
                if(cutdown)
                    cutdown = AdjustRightCutdown(node);
            }
            else
            {
                //hit
                Node *target = node;
                if (0==node->right)
                {
                    node = node->left;
                    cutdown = true;
                }
                else if(0==node->left)
                {
                    node = node->right;
                    cutdown = true;
                }
                else 
                {
                    Node *lf=node->left;
                    ReplaceRightMost(node,lf, cutdown);
                    node->left=lf;
                    if (cutdown)
                        cutdown = AdjustLeftCutdown(node);
                }
                --size_;
                delete target;
            }
        }

		struct Copier
		{
			AvlTree *to;
			template<typename T>
			void operator()(const T& content)
			{
				to->Insert(content);
			}
		};
    public:
        AvlTree()
            :size_(0)
        {
        }
        AvlTree(const AvlTree &copy)
            :size_(0)
        {
			*this=copy;
        }
		AvlTree& operator=(const AvlTree &copy)
		{
			Copier er={this};
			this->Clear();
			copy.Foreach(er);
			return *this;
		}
        int size()const
        {
            return size_;
        }
        template<typename N>
        T& Insert(const N &entity)
        {
            bool grow=false;
            return AddNode(this->root_,entity,grow);
        }
        template<typename N>
        void Erase(const N &entity)
        {
            bool cutdown=true;
            DeleteFromNode(this->root_,entity,cutdown);
        }
    private:
        int size_;
    };


    template<typename Key,typename Value>
    class OrderedMap
    {
    public:
        struct KeyValue
        {
            const Key key;
            Value value;
            // Insert で新規作成時.
            explicit KeyValue(const Key &key)
                :key(key)
            {
            }
            // Insert で Key が被ったときに呼ばれる.
            void operator=(const Key &)
            {
            }
			void operator=(const KeyValue &kv)
			{
				value=kv.value;
			}
        private:
            //DISALLOW_COPY_AND_ASSIGN(KeyValue);
        };
        struct Compare
        {
            inline bool Less(const KeyValue &a,const KeyValue &b)const
            {
                return a.key>b.key;
            }
            inline bool Greater(const KeyValue &a,const KeyValue &b)const
            {
                return a.key<b.key;
            }
            inline bool Less(const KeyValue &a,const Key &b)const
            {
                return a.key>b;
            }
            inline bool Greater(const KeyValue &a,const Key &b)const
            {
                return a.key<b;
            }
        };
        typedef typename AvlTree<KeyValue,Compare>::Iterator Iterator;
    public:
        OrderedMap()
        {

        }
		OrderedMap(const OrderedMap& copy)
		{
			*this=copy;
		}
        ~OrderedMap()
        {

        }
		OrderedMap& operator=(const OrderedMap& copy)
		{
			tree_=copy.tree_;
			return *this;
		}

        inline int Size()const
        {
            return tree_.size();
        }
        Value& operator[](const Key &key)
        {
            return tree_.Insert(key).value;
        }
        template<typename F>
        void Foreach(F& func)
        {
            tree_.Foreach(func);
        }
        KeyValue& Front()
        {
            return tree_.Front();
        }
        Value* Find(const Key &key)
        {
            KeyValue* kv=tree_.Find(key);
            return kv?&kv->value:0;
        }
        bool Has(const Key &key)const
        {
            return 0!=tree_.Find(key)?true:false;
        }
        void Erase(const Key &key)
        {
            return tree_.Erase(key);
        }
        AvlNode<KeyValue>* Root()
        {
            return tree_.Root();
        }
        void Clear()
        {
            tree_.Clear();
        }
    private:
        AvlTree<KeyValue,Compare> tree_;
    };


    template<typename T>
    class Allocated
    {
    public:
        T& operator*()
        {
            if(!enable_)
            {
                new(data_)T();
                enable_=true;
            }
            return *reinterpret_cast<T*>(data_);
        }
        T* operator->()
        {
            if(!enable_)
            {
                new(data_)T();
                enable_=true;
            }
            return reinterpret_cast<T*>(data_);
        }
        void Delete()
        {
            reinterpret_cast<T*>(data_)->~T();
            enable_=false;

        }
        Allocated()
        {
            enable_=false;
        }
        ~Allocated()
        {
            if(enable_)
            {
                reinterpret_cast<T*>(data_)->~T();
            }
        }
        operator bool()const
        {
            return enable_;
        }

    private:
        static const int SIZEOF=sizeof(T);
        bool enable_;
        char data_[SIZEOF];

        DISALLOW_COPY_AND_ASSIGN(Allocated);
    };


    template<typename T>
    struct DynamicPointer
    {
        T* ptr;
        DynamicPointer()
            :ptr(0){}
        DynamicPointer(T*p)
            :ptr(p){}
        ~DynamicPointer()
        {
            if(ptr)delete ptr;
        }
        T& operator*()
        {
            if(!ptr)
                ptr=new T;
            return *ptr;
        }
        T* operator->()
        {
            if(!ptr)
                ptr=new T;
            return ptr;                
        }
        operator bool()const
        {
            return ptr!=0;
        }
    private:
        DISALLOW_COPY_AND_ASSIGN(DynamicPointer);
    };

    template<typename T>
    class Manual
    {
    public:
        T& operator*()
        {
            return *reinterpret_cast<T*>(data_);
        }
        T* operator->()
        {
            return reinterpret_cast<T*>(data_);
        }
		void New()
		{
			new(data_)T();
		}
        void Delete()
        {
            reinterpret_cast<T*>(data_)->~T();
        }
    private:
        char data_[sizeof(T)];

		DISALLOW_COPY_AND_ASSIGN(Manual);
    };

}

template<typename ContentType>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,int n)
{
    if(n)
    {
        typename ContentType::T buf[13];
        bool minus;
        if(n<0)
        {
            minus=true;
            n*=-1;
        }
        else
        {
            minus=false;
        }
        int k=11;
        buf[12]=0;
        while(n)
        {
            buf[k--]='0'+(n%10);
            n/=10;
        }
        if(minus)
            buf[k--]='-';
        str+=buf+k+1;
        return str;
    }
    else
    {
        str.Append('0');
        return str;
    }
}

template<typename ContentType>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,const typename ContentType::T *n)
{
    str+=n;
    return str;
}

template<typename ContentType,typename ContentType2>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,const typename wts::BasicString<ContentType2> &n)
{
    str+=n.Data();
    return str;
}

template<typename ContentType>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,float n)
{
    const int m=64;

    struct sef
    {
        bool minus;
        int exp;
        int fraction;
        inline sef(float f)
            :minus((*(reinterpret_cast<uint32_t*>(&f))&0x80000000)?true:false)
            ,exp((0x7f800000&*(reinterpret_cast<uint32_t*>(&f)))>>23)
            ,fraction(0x007fffff&*(reinterpret_cast<uint32_t*>(&f)))
        {
        }
    };
    bool minus=sef(n).minus;
    if(minus)
        n*=-1;
    if(0==n)
    {
        str+="0.0";
        return str;
    }
    else if(-0.0f==n)
    {
        str+="-0.0";
        return str;
    }
    else if(255==sef(n).exp)
    {
        str+="NaN";
        return str;
    }
    typename ContentType::T num[m];

    int period=0;
    float f=n;
    for(;;)
    {
        int exp=sef(f).exp-127;
        if(exp<0)
        {
        }
        else
        {
            int frac=sef(f).fraction;
            frac<<=exp;
            if(0==(frac&0x7fffff))
                break;
        }
        f*=10;
        period++;
    }

    int k=m-1;
    num[k--]=0;
    if(0==period)
    {
        num[k--]='0';
    }
    while(0!=(int)f || 0<=period)
    {
        if(0==period--)
            num[k--]='.';
        num[k--]='0'+((int)f%10);
        f/=10;
    }
    if(minus)
        num[k--]='-';
    str+=num+k+1;
    return str;
}
/*
template<typename ContentType>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,float n)
{
    const int m=64;
    uint32_t ubit=*(reinterpret_cast<uint32_t*>(&n));
    bool minus=ubit&0x80000000?true:false;
    ubit&=0x7fffffff;
    int be=(0x7f800000&ubit)>>23;
    if(be==0)
    {
        str+="0.0";
        return str;
    }
    else if(be==255)
    {
        str+="NaN";
        return str;
    }
    typename ContentType::T num[m*2];
    num[m]='.';
    int leftk=m-1;
    if(n>1)
    {
        for(float f=n;f>=1;f/=10)
        {
            uint32_t bits=*reinterpret_cast<uint32_t*>(&f);
            int exp=((0x7f800000&bits)>>23)-127;
            if(exp<=23)
            {
                unsigned int frac=(0x007fffff&bits)|0x00800000;
                frac>>=(23-exp);
                num[leftk--]='0'+frac%10;
            }
            else
                num[leftk--]='0';
        }
    }
    else
        num[leftk--]='0';

    if(minus)
        num[leftk--]='-';
    int rightk=m+1;
    if(be-127>23)
    {
        num[rightk++]='0';
    }
    else
    {
        for(float f=n*10;;f*=10)
        {
            uint32_t bits=*reinterpret_cast<uint32_t*>(&f);
            int exp=((0x7f800000&bits)>>23)-127;
            if(exp<-8)
            {
                num[rightk++]='0';
            }
            else if(exp<=23)
            {
                unsigned int frac=(0x007fffff&bits)|0x00800000;
                frac>>=(23-exp);
                num[rightk++]='0'+frac%10;
            }
            else
                break;
        }
    }

    num[rightk]=0;

    str+=num+leftk+1;
    return str;
}*/

template<typename ContentType,typename AnyType>
wts::BasicString<ContentType>& operator<<(wts::BasicString<ContentType> &str,const typename wts::RawArray<AnyType> &n)
{
    for(int i=0;i<n.Size();i++)
        str<<n[i];
    return str;
}


#endif


