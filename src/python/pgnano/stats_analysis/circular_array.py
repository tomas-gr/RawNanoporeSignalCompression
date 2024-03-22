from typing import Any, List, Generic, TypeVar, Iterable

T = TypeVar('T')

class CircularArray(Generic[T]):
    def __init__(self, initial_values: List[T]):
        self.m_rep = initial_values.copy()
    
    def push(self, x: T) -> None:
        self.m_rep = self.m_rep[1:] + [x]

    def __getitem__(self, idx) -> T:
        return self.m_rep[idx]
    
    def copy(self):
        return CircularArray(self.m_rep)
    
    def get_indexes(self) -> Iterable[int]:
        return map(lambda x: x[0], enumerate(self.m_rep))
    
    def to_list(self) -> List[T]:
        return self.m_rep.copy()