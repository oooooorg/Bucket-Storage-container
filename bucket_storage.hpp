#ifndef BUCKET_STORAGE_HPP
#define BUCKET_STORAGE_HPP

#include <iostream>
#include <iterator>
#include <memory>

template< typename T >
class BucketStorage
{
  public:
	using size_type = std::size_t;
	using value_type = T;
	using reference = T&;
	using pointer = T*;
	using const_reference = const T&;
	using difference_type = std::ptrdiff_t;

	class iterator;
	class const_iterator;

	explicit BucketStorage(size_type cell_capacity = 64);
	BucketStorage(const BucketStorage& other);
	BucketStorage(BucketStorage&& other) noexcept;
	~BucketStorage();
	iterator insert(const T& value);
	iterator insert(T&& value);
	iterator erase(const iterator& iterator);
	bool empty() const noexcept;
	size_type size() const noexcept;
	size_type capacity() const noexcept;
	void shrink_to_fit() noexcept;
	void clear() noexcept;
	void swap(BucketStorage& other) noexcept;
	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	const_iterator cbegin() const noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;
	const_iterator cend() const noexcept;
	static iterator get_to_distance(iterator iterator, difference_type distance);
	BucketStorage& operator=(BucketStorage&& other) noexcept;
	BucketStorage& operator=(const BucketStorage& other);

  private:
	struct Cell
	{
		T* data{};
		size_type capacity;
		size_type size;
		bool* flags{};
		size_type deleted;

		explicit Cell(const size_type cap) : capacity(cap), size(0)
		{
			data = static_cast< T* >(operator new[](capacity * sizeof(T)));
			flags = new bool[capacity]();
		}

		~Cell()
		{
			for (size_type i = 0; i < size; ++i)
			{
				if (flags[i] == 0)
				{
					data[i].~T();
				}
			}
			operator delete[](data);
			delete[] flags;
		}
	};

	Cell** cells{};
	size_type cell_capacity{};
	size_type free_slots = 0;
	size_type cell_count{};
	size_type total_size{};

	void allocate_new_cell()
	{
		Cell** new_cells = new Cell*[cell_count + 1];
		for (size_type i = 0; i < cell_count; ++i)
		{
			new_cells[i] = cells[i];
		}
		new_cells[cell_count] = new Cell(cell_capacity);
		delete[] cells;
		cells = new_cells;
		++cell_count;
	}

	template< typename U >
	iterator insert_helper(U&& value)
	{
		if (total_size >= capacity())
		{
			allocate_new_cell();
		}
		for (size_type i = 0; i < cell_count; ++i)
		{
			Cell* cell = cells[i];
			if (cell_capacity > cell->size)
			{
				size_type index = cell->size;
				new (&cell->data[index]) value_type(std::forward< U >(value));
				cell->flags[index] = 0;
				++cell->size;
				total_size++;
				return iterator(this, i, index);
			}
		}
		return end();
	}

	size_type begin_helper(size_type& cell_index) const noexcept
	{
		cell_index = 0;
		while (cell_index < cell_count && cells[cell_index]->size == 0)
		{
			++cell_index;
		}

		if (cell_index >= cell_count)
		{
			return cell_count;
		}

		Cell* cell = cells[cell_index];
		size_type element_index = 0;
		while (element_index < cell->size && cell->flags[element_index] > 0)
		{
			++element_index;
		}

		return element_index;
	}

  public:
	class iterator
	{
	  public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;

	  private:
		size_type cell_index;
		size_type element_index;
		BucketStorage* storage;

		iterator(BucketStorage* storage, const size_type cell_index, const size_type element_index) :
			cell_index(cell_index), element_index(element_index), storage(storage)
		{
		}

	  public:
		iterator& operator++();
		iterator operator++(int);
		iterator& operator--();
		iterator operator--(int);
		bool operator==(const iterator& other) const;
		bool operator==(const const_iterator& other) const;
		bool operator!=(const iterator& other) const;
		bool operator!=(const const_iterator& other) const;
		T& operator*();
		T* operator->();
		bool operator>(const iterator& other) const;
		bool operator<(const iterator& other) const;
		bool operator>=(const iterator& other) const;
		bool operator<=(const iterator& other) const;

		friend class BucketStorage;
	};

	class const_iterator
	{
	  public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;

	  private:
		size_type cell_index;
		size_type element_index;
		const BucketStorage* storage;

		const_iterator(const BucketStorage* storage, const size_type cell_index, const size_type element_index) :
			cell_index(cell_index), element_index(element_index), storage(storage)
		{
		}

	  public:
		const_iterator& operator++();
		const_iterator operator++(int);
		bool operator==(const iterator& other) const;
		bool operator==(const const_iterator& other) const;
		bool operator!=(const iterator& other) const;
		bool operator!=(const const_iterator& other) const;
		const T& operator*() const;

		friend class BucketStorage;
	};
};

template< typename T >
BucketStorage< T >::BucketStorage(const size_type cell_capacity) :
	cells(new Cell*[cell_capacity]), cell_capacity(cell_capacity)
{
}

template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage& other) :
	cell_capacity(other.cell_capacity), cell_count(other.cell_count), total_size(other.total_size)
{
	cells = new Cell*[cell_count];
	for (size_type i = 0; i < cell_count; ++i)
	{
		cells[i] = new Cell(cell_capacity);
	}

	for (auto i = other.begin(); i != other.end(); ++i)
	{
		size_type cell_index = i.cell_index;
		size_type element_index = cells[cell_index]->size;

		if (element_index < cell_capacity)
		{
			new (&cells[cell_index]->data[element_index]) value_type(*i);
			cells[cell_index]->flags[element_index] = 0;
			++cells[cell_index]->size;
		}
	}
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage&& other) noexcept
{
	swap(other);
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(BucketStorage&& other) noexcept
{
	if (this != &other)
	{
		clear();
		swap(other);
	}
	return *this;
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(const BucketStorage& other)
{
	if (this != &other)
	{
		clear();
		BucketStorage copy(other);
		swap(copy);
	}
	return *this;
}

template< typename T >
void BucketStorage< T >::clear() noexcept
{
	for (size_type i = 0; i < cell_count; ++i)
	{
		delete cells[i];
	}
	delete[] cells;
	cells = nullptr;
	cell_count = 0;
	total_size = 0;
}

template< typename T >
BucketStorage< T >::~BucketStorage()
{
	clear();
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(const value_type& value)
{
	return insert_helper(value);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(value_type&& value)
{
	return insert_helper(std::move(value));
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::erase(const iterator& iterator)
{
	if (iterator.storage != this || iterator.cell_index >= cell_count)
	{
		return end();
	}

	Cell* cell = cells[iterator.cell_index];
	size_type index = iterator.element_index;

	if (index < cell->size && cell->flags[index] == 0)
	{
		cell->data[index].~T();
		cell->flags[index] = 1;
		--total_size;

		for (size_type i = index; i < cell->size - 1; ++i)
		{
			if (cell->flags[i + 1] == 0)
			{
				cell->data[i] = std::move(cell->data[i + 1]);
				cell->flags[i] = cell->flags[i + 1];
			}
		}

		--cell->size;

		if (cell->size == 0)
		{
			delete cell;
			for (size_type i = iterator.cell_index; i < cell_count - 1; ++i)
			{
				cells[i] = cells[i + 1];
			}
			--cell_count;
		}
	}

	return end();
}

template< typename T >
bool BucketStorage< T >::empty() const noexcept
{
	return total_size == 0;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::size() const noexcept
{
	return total_size;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::capacity() const noexcept
{
	return cell_capacity * cell_count;
}

template< typename T >
void BucketStorage< T >::shrink_to_fit() noexcept
{
	if (total_size == 0)
	{
		clear();
		return;
	}

	const size_type new_cell_count = (total_size + cell_capacity - 1) / cell_capacity;

	if (new_cell_count >= cell_count)
	{
		return;
	}

	Cell** new_cells = new Cell*[new_cell_count];
	for (size_type i = 0; i < new_cell_count; ++i)
	{
		new_cells[i] = new Cell(cell_capacity);
	}

	size_type current_index = 0;
	for (size_type i = 0; i < cell_count; ++i)
	{
		Cell* cell = cells[i];

		for (size_type j = 0; j < cell->size; ++j)
		{
			if (cell->flags[j] == 0)
			{
				size_type new_cell_index = current_index / cell_capacity;
				size_type new_element_index = current_index % cell_capacity;

				new (&new_cells[new_cell_index]->data[new_element_index]) T(std::move(cell->data[j]));
				new_cells[new_cell_index]->flags[new_element_index] = 0;
				++new_cells[new_cell_index]->size;
				++current_index;
			}
		}
	}

	for (size_type i = 0; i < cell_count; ++i)
	{
		delete cells[i];
	}
	delete[] cells;

	cells = new_cells;
	cell_count = new_cell_count;
	total_size = current_index;
}

template< typename T >
void BucketStorage< T >::swap(BucketStorage& other) noexcept
{
	using std::swap;
	swap(cells, other.cells);
	swap(cell_capacity, other.cell_capacity);
	swap(cell_count, other.cell_count);
	swap(total_size, other.total_size);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	size_type cell_index;
	size_type element_index = begin_helper(cell_index);
	return iterator(this, cell_index, element_index);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	size_type cell_index;
	size_type element_index = begin_helper(cell_index);
	return const_iterator(this, cell_index, element_index);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() const noexcept
{
	return begin();
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(this, cell_count, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(this, cell_count, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() const noexcept
{
	return end();
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::get_to_distance(iterator iterator, const difference_type distance)
{
	if (difference_type dist = distance; dist > 0)
	{
		while (dist)
		{
			++iterator;
			--dist;
		}
	}
	else
	{
		while (dist < 0)
		{
			--iterator;
			++dist;
		}
	}

	return iterator;
}

template< typename T >
typename BucketStorage< T >::iterator& BucketStorage< T >::iterator::operator++()
{
	size_type next_index = ++element_index;
	const Cell* cell = storage->cells[cell_index];

	while (next_index < cell->size && cell->flags[next_index])
	{
		next_index++;
	}

	if (next_index < cell->size)
	{
		element_index = next_index;
	}
	else
	{
		while (++cell_index < storage->cell_count)
		{
			cell = storage->cells[cell_index];
			if (cell->size && cell->flags[0] == 0)
			{
				element_index = 0;
				break;
			}
		}
	}

	if (cell_index >= storage->cell_count || element_index >= cell->size)
	{
		cell_index = storage->cell_count;
		element_index = 0;
	}

	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::iterator::operator++(int)
{
	iterator helper = *this;
	++(*this);
	return helper;
}

template< typename T >
typename BucketStorage< T >::const_iterator& BucketStorage< T >::const_iterator::operator++()
{
	size_type next_index = ++element_index;
	const Cell* cell = storage->cells[cell_index];

	while (next_index < cell->size && cell->flags[next_index])
	{
		next_index++;
	}

	if (next_index < cell->size)
	{
		element_index = next_index;
	}
	else
	{
		while (++cell_index < storage->cell_count)
		{
			cell = storage->cells[cell_index];
			if (cell->size && cell->flags[0] == 0)
			{
				element_index = 0;
				break;
			}
		}
	}

	if (cell_index >= storage->cell_count || element_index >= cell->size)
	{
		cell_index = storage->cell_count;
		element_index = 0;
	}

	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::const_iterator::operator++(int)
{
	const_iterator helper = *this;
	++(*this);
	return helper;
}

template< typename T >
typename BucketStorage< T >::iterator& BucketStorage< T >::iterator::operator--()
{
	if (element_index)
	{
		--element_index;
		while (element_index && storage->cells[cell_index]->flags[element_index])
		{
			--element_index;
		}
	}
	else
	{
		while (--cell_index >= 0)
		{
			Cell* cell = storage->cells[cell_index];
			if (cell->size && cell->flags[cell->size - 1] == 0)
			{
				element_index = cell->size - 1;
				break;
			}
		}
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::iterator::operator--(int)
{
	iterator helper = *this;
	--(*this);
	return helper;
}

template< typename T >
bool BucketStorage< T >::iterator::operator==(const iterator& other) const
{
	return storage == other.storage && cell_index == other.cell_index && element_index == other.element_index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator==(const const_iterator& other) const
{
	return storage == other.storage && cell_index == other.cell_index && element_index == other.element_index;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator==(const iterator& other) const
{
	return storage == other.storage && cell_index == other.cell_index && element_index == other.element_index;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator==(const const_iterator& other) const
{
	return storage == other.storage && cell_index == other.cell_index && element_index == other.element_index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator!=(const const_iterator& other) const
{
	return !(*this == other);
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

template< typename T >
typename BucketStorage< T >::const_reference BucketStorage< T >::const_iterator::operator*() const
{
	return storage->cells[cell_index]->data[element_index];
}

template< typename T >
bool BucketStorage< T >::iterator::operator>(const iterator& other) const
{
	return !(other == *this) &&
		   (cell_index > other.cell_index || (cell_index == other.cell_index && element_index > other.element_index));
}

template< typename T >
bool BucketStorage< T >::iterator::operator<(const iterator& other) const
{
	return !(*this > other) && !(*this == other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator>=(const iterator& other) const
{
	return !(*this < other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator<=(const iterator& other) const
{
	return !(*this > other);
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator!=(const const_iterator& other) const
{
	return !(*this == other);
}

template< typename T >
typename BucketStorage< T >::reference BucketStorage< T >::iterator::operator*()
{
	return storage->cells[cell_index]->data[element_index];
}

template< typename T >
typename BucketStorage< T >::pointer BucketStorage< T >::iterator::operator->()
{
	return &storage->cells[cell_index]->data[element_index];
}

#endif	  // BUCKET_STORAGE_HPP
