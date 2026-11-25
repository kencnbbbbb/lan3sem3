package ds

import "errors"

var (
	ErrOutOfRange = errors.New("индекс вне диапазона")
	ErrEmpty      = errors.New("структура пуста")
	ErrNotFound   = errors.New("элемент не найден")
	ErrNoNeighbor = errors.New("нет соседнего элемента")
	ErrCapacity   = errors.New("превышена максимальная ёмкость")
	ErrFormat     = errors.New("неверный формат данных")
)
