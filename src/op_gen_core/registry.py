from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

from .errors import SchemaError


@dataclass
class TypeInfo:
    name: str
    kind: str
    rendered: str


class TypeRegistry:
    """Tracks rendered C++ types so later kinds can reference earlier ones."""

    def __init__(self) -> None:
        self._m: Dict[str, TypeInfo] = {}

    def add(self, name: str, *, kind: str, rendered: str) -> None:
        self._m[name] = TypeInfo(name=name, kind=kind, rendered=rendered)

    def get(self, name: str) -> TypeInfo:
        if name not in self._m:
            raise SchemaError(f"Unknown type reference '{name}'")
        return self._m[name]


class BaseOpGenerator:
    """Base class for language generators (currently C++).

    Concrete generators implement one or both methods.
    """

    def emit_config_ops(self, **kwargs: Any) -> None:  # pragma: no cover
        raise NotImplementedError

    def emit_type_ops(self, **kwargs: Any) -> None:  # pragma: no cover
        raise NotImplementedError

