#include <cstddef>
#include <iterator>
#include <ostream>
#include "abstract_iterator.h"

namespace bmstu
{
template <typename T>
class list
{
	struct node
	{
		node() = default;

		node(node* prev, const T& value, node* next)
			: next_node_(next), prev_node_(prev), value_(value)
		{
		}

		T value_;
		node* next_node_ = nullptr;
		node* prev_node_ = nullptr;
	};

   public:
	struct iterator
		: public abstract_iterator<iterator, T, std::bidirectional_iterator_tag>
	{
		node* current;
		iterator() : current(nullptr) {}
		iterator(node* node) : current(node) {}
		iterator& operator++() override
		{
			if (current != nullptr)
			{
				current = current->next_node_;
			}
			return *this;
		}
		iterator& operator--() override
		{
			if (current != nullptr)
			{
				current = current->prev_node_;
			}
			return *this;
		}
		iterator operator++(int) override
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}
		iterator operator--(int) override
		{
			iterator temp = *this;
			--(*this);
			return temp;
		}
		iterator& operator+=(
			const typename abstract_iterator<
				iterator,
				T,
				std::bidirectional_iterator_tag>::difference_type& n) override
		{
			for (auto i = 0; i < n; ++i)
			{
				++(*this);
			}
			return *this;
		}
		iterator& operator-=(
			const typename abstract_iterator<
				iterator,
				T,
				std::bidirectional_iterator_tag>::difference_type& n) override
		{
			for (auto i = 0; i < n; ++i)
			{
				--(*this);
			}
			return *this;
		}
		iterator operator+(const typename abstract_iterator<
						   iterator,
						   T,
						   std::bidirectional_iterator_tag>::difference_type& n)
			const override
		{
			iterator result = *this;
			result += n;

			return result;
		}
		iterator operator-(const typename abstract_iterator<
						   iterator,
						   T,
						   std::bidirectional_iterator_tag>::difference_type& n)
			const override
		{
			iterator result = *this;
			result -= n;

			return result;
		}
		typename abstract_iterator<iterator,
								   T,
								   std::bidirectional_iterator_tag>::reference
		operator*() const override
		{
			return current->value_;
		}
		typename abstract_iterator<iterator,
								   T,
								   std::bidirectional_iterator_tag>::pointer
		operator->() const override
		{
			return &(current->value_);
		}
		bool operator==(const iterator& other) const override
		{
			return current == other.current;
		}
		bool operator!=(const iterator& other) const override
		{
			return current != other.current;
		}
		explicit operator bool() const override { return current != nullptr; }
		typename abstract_iterator<
			iterator,
			T,
			std::bidirectional_iterator_tag>::difference_type
		operator-(const iterator& other) const override
		{
			typename abstract_iterator<
				iterator, T, std::bidirectional_iterator_tag>::difference_type
				count = 0;
			iterator it = other;
			while (it != *this)
			{
				++it;
				++count;
			}
			return count;
		}
	};
	using const_iterator = iterator;

	list()
	{
		head_ = new node();
		tail_ = new node();

		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;
		size_ = 0;
	}

	template <typename it>
	list(it begin, it end) : list()
	{
		for (auto it_pos = begin; it_pos != end; ++it_pos)
		{
			push_back(*it_pos);
		}
	}

	list(std::initializer_list<T> values) : list()
	{
		for (const auto& value : values)
		{
			push_back(value);
		}
	}

	list(const list& other) : list()
	{
		for (const auto& value : other)
		{
			push_back(value);
		}
	}

	list(list&& other) : list() { swap(other); }

#pragma endregion
#pragma region pushs

	template <typename Type>
	void push_back(const Type& value)
	{
		insert(end(), value);
	}

	template <typename Type>
	void push_front(const Type& value)
	{
		insert(begin(), value);
	}

	void pop_front() { erase(begin()); }

	void pop_back() { erase(end() - 1); }

	iterator erase(const_iterator& pos)
	{
		node* current_node = const_cast<node*>(pos.current);

		node* next = current_node->next_node_;
		node* prev_current = current_node->prev_node_;

		prev_current->next_node_ = next;
		next->prev_node_ = prev_current;

		delete current_node;

		--size;

		return iterator{next};
	}

#pragma endregion

	bool empty() const

		noexcept
	{
		return (size_ == 0u);
	}

	~list()
	{
		clear();
		delete tail_;
		delete head_;
	}

	void clear()
	{
		node* current = head_->next_node_;
		while (current != tail_)
		{
			node* del = current;
			current = current->next_node_;
			delete del;
		}
		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;
		size_ = 0;
	}

	size_t size() const { return size_; }

	void swap(list& other)

		noexcept
	{
		std::swap(tail_, other.tail_);
		std::swap(head_, other.head_);
		std::swap(size_, other.size_);
	}

	friend void swap(list& l, list& r) { l.swap(r); }

#pragma region iterators

	iterator begin()

		noexcept
	{
		return iterator{head_->next_node_};
	}

	iterator end()

		noexcept
	{
		return iterator{tail_};
	}

	const_iterator begin() const

		noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator end() const

		noexcept
	{
		return const_iterator{tail_};
	}

	const_iterator cbegin() const

		noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator cend() const

		noexcept
	{
		return const_iterator{tail_};
	}

#pragma endregion

	T operator[](size_t pos) const
	{
		iterator it = begin();
		for (size_t i = 0; i < pos; ++i)
		{
			++it;
		}
		return *it;
	}

	T& operator[](size_t pos)
	{
		iterator it = begin();
		for (size_t i = 0; i < pos; i++)
		{
			++it;
		}
		return *it;
	}

	friend bool operator==(const list& l, const list& r)
	{
		if (l.size_ != r.size_)
		{
			return false;
		}
		auto lit = l.begin();
		auto rit = r.begin();
		while (lit != l.end() && rit != r.end())
		{
			if (*lit != *rit)
			{
				return false;
			}
			++lit;
			++rit;
		}
		return true;
	}

	friend bool operator!=(const list& l, const list& r) { return !(l == r); }

	friend auto operator<=>(const list& lhs, const list& rhs)
	{
		auto lit = lhs.begin();
		auto rit = rhs.begin();

		while (rit != lhs.end() && rit != rhs.end())
		{
			if (*lit != *rit)
			{
				return *lit <=> *rit;
			}
			++lit;
			++rit;
		}
		return lhs.size() <=> rhs.size();
	}

	friend std::ostream& operator<<(std::ostream& os, const list& other)
	{
		os << "{";
		for (auto it = other.begin(); it != other.end(); ++it)
		{
			if (it != other.begin())
			{
				os << ", ";
			}
			os << *it;
		}
		os << "}";
		return os;
	}

	iterator insert(const_iterator pos, const T& value)
	{
		node* current_node = pos.current;
		node* prev_node = current_node->prev_node_;

		node* new_node = new node(prev_node, value, current_node);
		prev_node->next_node_ = new_node;
		current_node->prev_node_ = new_node;
		++size_;

		return iterator{new_node};
	}

   private:
	static bool lexicographical_compare_(const list<T>& l, const list<T>& r)
	{
		auto lit = l.begin();
		auto rit = r.begin();
		while (lit != l.end() && rit != r.end())
		{
			if (*lit < *rit)
			{
				return true;
			}
			if (*lit > *rit)
			{
				return false;
			}
			++lit;
			++rit;
		}
		return (lit == l.end()) && (rit != r.end());
	}

	size_t size_ = 0;
	node* tail_ = nullptr;
	node* head_ = nullptr;
};
}  // namespace bmstu