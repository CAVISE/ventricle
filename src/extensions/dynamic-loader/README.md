The C-compatible interface lives in `model/extension.h`. Each shared extension
exports `vcleGetExtensionInfo()`, returning a library-owned `VcleExtensionInfo`.
The loader checks `apiVersion` before invoking `registerTypes`. That callback
registers its ns-3 types and reports each public type's name through the host's
`VcleRegisterType` callback and opaque context. Other registered types remain
internal. Component construction still uses helpers or ns-3 factories where supported.

Both registration callbacks return zero on success. Extensions must propagate
host callback failures and prevent exceptions from crossing the C interface.
Callback arguments are borrowed for the duration of registration. The host
resolves names against its ns-3 registry and stores the resulting TypeIds and
copied metadata in a shared `Extension` object, independent of the loader lifetime.

`Loader::load(path)` loads an explicit library path and validates its entrypoint
and name. It uses POSIX `dlopen` (Linux/macOS). ns-3 component libraries and their dependencies
must be built with the same C++ ABI and shared ns-3 runtime as the host. The
descriptor API version identifies the C interface contract; it does not establish
binary compatibility for the ns-3 C++ implementation behind it.

Loading is intended for single-threaded setup. Libraries remain resident until
process exit, including rejected libraries: static initialization may already
have registered accessors or constructors in ns-3. There is no unload operation.
