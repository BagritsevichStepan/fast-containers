#ifndef FAST_CONTAINERS_INTRUSIVE_D_HEAP_H
#define FAST_CONTAINERS_INTRUSIVE_D_HEAP_H

namespace fast_containers {

    class DefaultDHeapElementTag;

    template <typename Tag = DefaultDHeapElementTag>
    class DHeapElement;


    namespace details::intrusive_d_heap {

        template <typename Element, typename Tag>
        concept IsDHeapElement = std::is_base_of_v<DHeapElement<Tag>, Element>;

    } // End of namespace fast_containers::details::intrusive_d_heap


    template<typename Element, typename Tag = DefaultDHeapElementTag, typename Comparator = std::less<Element>>
    requires details::intrusive_d_heap::IsDHeapElement<Element, Tag>
    class IntrusiveDHeap;


    class DHeapElementBase {
    protected:
        DHeapElementBase() = default;
        ~DHeapElementBase() = default;

    public:
        DHeapElementBase(const DHeapElementBase&) = delete;
        DHeapElementBase(DHeapElementBase&&) = delete;
        DHeapElementBase& operator=(const DHeapElementBase&) = delete;
        DHeapElementBase& operator=(DHeapElementBase&&) = delete;

    private:
        template<typename Element, typename Tag, typename Comparator>
        requires details::intrusive_d_heap::IsDHeapElement<Element, Tag>
        class IntrusiveDHeap;

        DHeapElementBase* parent_{this}; //todo
        DHeapElementBase* left_child_{this}; //todo
        DHeapElementBase* right_neighbor_{this}; //todo

    };

    template<typename Tag>
    class DHeapElement : private DHeapElementBase {
    protected:
        DHeapElement() = default;
        ~DHeapElement() = default;

    public:
        DHeapElement(const DHeapElement&) = delete;
        DHeapElement(DHeapElement&&) = delete;
        DHeapElement& operator=(const DHeapElement&) = delete;
        DHeapElement& operator=(DHeapElement&&) = delete;

    private:
        template<typename Element, typename Comparator>
        requires details::intrusive_d_heap::IsDHeapElement<Element, Tag>
        class IntrusiveDHeap;

    };

    template<typename Element, typename Tag, typename Comparator>
    requires details::intrusive_d_heap::IsDHeapElement<Element, Tag>
    class IntrusiveDHeap {


    private:
        void SiftUp();

        void SiftDown();
    };

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_INTRUSIVE_D_HEAP_H
