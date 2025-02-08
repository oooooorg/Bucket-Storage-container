# Лабораторная работа №3. STL-совместимый контейнер

Текущий статус тестирования GitHub Actions: [![CI/CD](../../actions/workflows/classroom.yml/badge.svg?branch=main&event=workflow_dispatch)](../../actions/workflows/classroom.yml).

> [!CAUTION]
> Изменять код в [`tests.cpp`](tests.cpp) и [`helpers.hpp`](helpers.hpp) **запрещено**. Интерфейс и реализация разрабатываемого класса прописывается в [`bucket_storage.hpp`](bucket_storage.hpp).
>
> [**Не забудьте инициализировать репозиторий!**](#инициализация-репозитория)

## Инициализация репозитория

После *первого клонирования* репозитория его необходимо инициализировать. Для этого Вам необходимо запустить соответствующий скрипт, для запуска требуется [Bash](https://en.wikipedia.org/wiki/Bash_(Unix_shell)) (есть в UNIX-подобных системах, в Windows – подойдёт [Git Bash](https://git-scm.com/)) и [GitHub CLI](https://cli.github.com/):

```bash
bash init-repo.sh
```

Скрипт необходимо запустить **только один раз**.

## Локальное тестирование

Вам предоставляется возможность запуска тестов локальным способом. Для этого нужно:

1. Поставить [GoogleTest](https://skkv-itmo.gitbook.io/ct-c-cpp-course/testing/gtest).
2. *(опционально)* В среде разработки для удобства следует разобраться с обозревателем тестов и тем, как создать проект с поддержкой `gtest`.

На сервере используется версия [`1.14.0`](https://github.com/google/googletest/tree/v1.14.0).

## Требования к оформлению класса

Сначала описываются интерфейсы шаблонных классов, затем – реализация их методов.

Пример оформления:

```cpp
// ------------------------------------
// START OF INTERFACE
// ------------------------------------

template< typename T >
class BucketStorage
{
  public:
    BucketStorage();
    BucketStorage(const BucketStorage &other);
    BucketStorage(BucketStorage &&other);

    ~BucketStorage();

  private:
};

// ------------------------------------
// START OF IMPLEMENTATION
// ------------------------------------

template< typename T >
BucketStorage< T >::BucketStorage()
{
}

template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage &other)
{
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage &&other)
{
}

template< typename T >
BucketStorage< T >::~BucketStorage()
{
}
```
