#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bmstu
{

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class unordered_map
{
   private:
	struct node
	{
		std::pair<const Key, Value> value;
		node* next = nullptr;

		node(const Key& key, const Value& val) : value(key, val), next(nullptr)
		{
		}
		node(Key&& key, Value&& val)
			: value(std::move(key), std::move(val)), next(nullptr)
		{
		}
	};

	using bucket_type = node*;

	std::vector<bucket_type> buckets_;
	size_t size_ = 0;
	Hash hasher_;

	static constexpr float MAX_LOAD_FACTOR = 0.75f;

	size_t get_bucket_index(const Key& key) const
	{
		return hasher_(key) % buckets_.size();
	}

	void rehash(size_t new_bucket_count)
	{
		std::vector<bucket_type> new_buckets(new_bucket_count, nullptr);

		for (size_t i = 0; i < buckets_.size(); i++)
		{
			node* current = buckets_[i];
			while (current != nullptr)
			{
				node* next = current->next;
				size_t new_index =
					hasher_(current->value.first) % new_bucket_count;

				current->next = new_buckets[new_index];
				new_buckets[new_index] = current;
				current = next;
			}
		}

		buckets_.swap(new_buckets);
	}

	void check_and_rehash()
	{
		if (static_cast<float>(size_) / buckets_.size() > MAX_LOAD_FACTOR)
		{
			rehash(buckets_.size() * 2);
		}
	}

	size_t find_next_non_empty_bucket(size_t start) const
	{
		for (size_t i = start; i < buckets_.size(); i++)
		{
			if (buckets_[i] != nullptr)
			{
				return i;
			}
		}
		return buckets_.size();
	}

   public:
	class iterator
	{
	   private:
		node* current_node = nullptr;
		const std::vector<bucket_type>* buckets_ptr = nullptr;
		size_t current_bucket_index = 0;

		void advance_to_next_valid()
		{
			if (current_node != nullptr && current_node->next != nullptr)
			{
				current_node = current_node->next;
				return;
			}

			if (buckets_ptr != nullptr)
			{
				for (size_t i = current_bucket_index + 1;
					 i < buckets_ptr->size(); i++)
				{
					if ((*buckets_ptr)[i] != nullptr)
					{
						current_node = (*buckets_ptr)[i];
						current_bucket_index = i;
						return;
					}
				}
			}

			current_node = nullptr;
			current_bucket_index = 0;
		}

	   public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<const Key, Value>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		iterator() = default;

		iterator(node* node,
				 const std::vector<bucket_type>* buckets,
				 size_t bucket_idx)
			: current_node(node),
			  buckets_ptr(buckets),
			  current_bucket_index(bucket_idx)
		{
		}

		reference operator*() const { return current_node->value; }

		pointer operator->() const { return &(current_node->value); }

		iterator& operator++()
		{
			advance_to_next_valid();
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(const iterator& other) const
		{
			return current_node == other.current_node;
		}

		bool operator!=(const iterator& other) const
		{
			return current_node != other.current_node;
		}

		explicit operator bool() const { return current_node != nullptr; }
	};

	class const_iterator
	{
	   private:
		const node* current_node = nullptr;
		const std::vector<bucket_type>* buckets_ptr = nullptr;
		size_t current_bucket_index = 0;

		void advance_to_next_valid()
		{
			if (current_node != nullptr && current_node->next != nullptr)
			{
				current_node = current_node->next;
				return;
			}

			if (buckets_ptr != nullptr)
			{
				for (size_t i = current_bucket_index + 1;
					 i < buckets_ptr->size(); i++)
				{
					if ((*buckets_ptr)[i] != nullptr)
					{
						current_node = (*buckets_ptr)[i];
						current_bucket_index = i;
						return;
					}
				}
			}

			current_node = nullptr;
			current_bucket_index = 0;
		}

	   public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = const std::pair<const Key, Value>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		const_iterator() = default;

		const_iterator(const node* node,
					   const std::vector<bucket_type>* buckets,
					   size_t bucket_idx)
			: current_node(node),
			  buckets_ptr(buckets),
			  current_bucket_index(bucket_idx)
		{
		}

		const_iterator(const iterator& it)
			: current_node(it.current_node),
			  buckets_ptr(it.buckets_ptr),
			  current_bucket_index(it.current_bucket_index)
		{
		}

		reference operator*() const { return current_node->value; }
		pointer operator->() const { return &(current_node->value); }

		const_iterator& operator++()
		{
			advance_to_next_valid();
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(const const_iterator& other) const
		{
			return current_node == other.current_node;
		}

		bool operator!=(const const_iterator& other) const
		{
			return current_node != other.current_node;
		}
	};

	unordered_map() : buckets_(16, nullptr) {}

	explicit unordered_map(size_t initial_buckets)
		: buckets_(initial_buckets, nullptr)
	{
	}

	unordered_map(std::initializer_list<std::pair<const Key, Value>> init)
		: unordered_map()
	{
		for (const auto& pair : init)
		{
			insert(pair.first, pair.second);
		}
	}

	template <typename InputIt>
	unordered_map(InputIt first, InputIt last) : unordered_map()
	{
		for (auto it = first; it != last; ++it)
		{
			insert(it->first, it->second);
		}
	}

	unordered_map(const unordered_map& other)
		: buckets_(other.buckets_.size(), nullptr), size_(0)
	{
		for (const auto& pair : other)
		{
			insert(pair.first, pair.second);
		}
	}

	unordered_map(unordered_map&& other) noexcept
		: buckets_(std::move(other.buckets_)),
		  size_(other.size_),
		  hasher_(std::move(other.hasher_))
	{
		other.size_ = 0;
		other.buckets_.clear();
	}

	~unordered_map() { clear(); }

	unordered_map& operator=(const unordered_map& other)
	{
		if (this != &other)
		{
			unordered_map temp(other);
			swap(temp);
		}
		return *this;
	}

	unordered_map& operator=(unordered_map&& other) noexcept
	{
		if (this != &other)
		{
			clear();
			buckets_ = std::move(other.buckets_);
			size_ = other.size_;
			hasher_ = std::move(other.hasher_);
			other.size_ = 0;
			other.buckets_.clear();
		}
		return *this;
	}

	void swap(unordered_map& other) noexcept
	{
		std::swap(buckets_, other.buckets_);
		std::swap(size_, other.size_);
		std::swap(hasher_, other.hasher_);
	}

	friend void swap(unordered_map& l, unordered_map& r) { l.swap(r); }

	void insert(const Key& key, const Value& value)
	{
		if (buckets_.empty())
		{
			rehash(16);
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				current->value.second = value;
				return;
			}
			current = current->next;
		}

		node* new_node = new node(key, value);
		new_node->next = buckets_[index];
		buckets_[index] = new_node;
		++size_;
		check_and_rehash();
	}

	void insert(Key&& key, Value&& value)
	{
		if (buckets_.empty())
		{
			rehash(16);
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				current->value.second = std::move(value);
				return;
			}
			current = current->next;
		}

		node* new_node = new node(std::move(key), std::move(value));
		new_node->next = buckets_[index];
		buckets_[index] = new_node;
		++size_;
		check_and_rehash();
	}

	Value& operator[](const Key& key)
	{
		if (buckets_.empty())
		{
			rehash(16);
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				return current->value.second;
			}
			current = current->next;
		}

		node* new_node = new node(key, Value());
		new_node->next = buckets_[index];
		buckets_[index] = new_node;
		++size_;
		check_and_rehash();

		return new_node->value.second;
	}

	const Value& at(const Key& key) const
	{
		auto it = find(key);
		if (it == end())
		{
			throw std::out_of_range("Key not found");
		}
		return it->second;
	}

	void erase(const Key& key)
	{
		if (buckets_.empty())
		{
			return;
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];
		node* prev = nullptr;

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				if (prev == nullptr)
				{
					buckets_[index] = current->next;
				}
				else
				{
					prev->next = current->next;
				}
				delete current;
				--size_;
				return;
			}
			prev = current;
			current = current->next;
		}
	}

	iterator find(const Key& key)
	{
		if (buckets_.empty())
		{
			return end();
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				return iterator(current, &buckets_, index);
			}
			current = current->next;
		}

		return end();
	}

	const_iterator find(const Key& key) const
	{
		if (buckets_.empty())
		{
			return end();
		}

		size_t index = get_bucket_index(key);
		node* current = buckets_[index];

		while (current != nullptr)
		{
			if (current->value.first == key)
			{
				return const_iterator(current, &buckets_, index);
			}
			current = current->next;
		}

		return end();
	}

	bool contains(const Key& key) const { return find(key) != end(); }

	size_t size() const noexcept { return size_; }
	bool empty() const noexcept { return size_ == 0; }
	size_t bucket_count() const noexcept { return buckets_.size(); }

	void clear()
	{
		for (size_t i = 0; i < buckets_.size(); i++)
		{
			node* current = buckets_[i];
			while (current != nullptr)
			{
				node* to_delete = current;
				current = current->next;
				delete to_delete;
			}
			buckets_[i] = nullptr;
		}
		size_ = 0;
	}

	iterator begin()
	{
		for (size_t i = 0; i < buckets_.size(); ++i)
		{
			if (buckets_[i] != nullptr)
			{
				return iterator(buckets_[i], &buckets_, i);
			}
		}
		return end();
	}

	iterator end() { return iterator(nullptr, &buckets_, buckets_.size()); }

	const_iterator begin() const
	{
		for (size_t i = 0; i < buckets_.size(); ++i)
		{
			if (buckets_[i] != nullptr)
			{
				return const_iterator(buckets_[i], &buckets_, i);
			}
		}
		return end();
	}

	const_iterator end() const
	{
		return const_iterator(nullptr, &buckets_, buckets_.size());
	}

	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

	friend bool operator==(const unordered_map& lhs, const unordered_map& rhs)
	{
		if (lhs.size_ != rhs.size_)
		{
			return false;
		}

		for (auto it = lhs.cbegin(); it != lhs.cend(); ++it)
		{
			auto rit = rhs.find(it->first);
			if (rit == rhs.cend() || !(rit->second == it->second))
			{
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const unordered_map& lhs, const unordered_map& rhs)
	{
		return !(lhs == rhs);
	}
};

}  // namespace bmstu