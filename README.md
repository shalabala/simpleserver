## Simple C-based MVC framework
This is a little two week project of mine, which mimics MVC frameworks like ASP.NET MVC or Spring Boot MVC, but this one is entirely written in C.

### Getting it to run
I only tested it on Linux. To start it simply compile the project with `make clean simpleserver` and then run it with `./simpleserver`. With VS Code the project can be run by pressing F5.

### Controllers
To define your own pages you need to register a new controller in the controller.c/.h files. A controller receives:
- a `request` pointer: this contains information about the request, like query parameters, method, resource string and header values
- an `smap *urlargs`: this contains all parts of the url that were mapped by the controller registration. These are __not__ the query parameters. Example: if the controller is registered for the path "/user/{id}" and the HTTP request is made for "/users/7" then `urlargs` will have the mapping `"id"->"7"`
- a `response`pointer: this needs to be filled by the controller. Useful helper methods are `create_html` `set_resp_code` and `notfound`
- a `context` pointer: this also needs to be filled for the template parameters that is used to render the html body.

The actual controller registrations are done inside the `reg_controllers` method. `**` matches any path that does not have a more specific controller. `*` matches until the next `/` character. With `{..}` parts of the URL can be forwarded to the controller as named parameters. More specific controllers always match paths better than less specific ones.

### Templating
The html files that are passed to `create_html` also support rudimentary templating. Following constructs can be used:
- `{{var}}` or `{{collection[i]}}`: simple variable interpolation from the context. If [] is used the contents of the indexer are also resolved from the context.
- `{{if var1}} .. {{elif var2}} .. {{else}} .. {{endif}}`: the vars need to evaluate either to `true` or to `false`
- `{{for e in 1..10}}` or `{{for e in 1..10/2}}` or `{{for e in vars}}`: for each capabilities. If `vars` is used (collection) the elements need to be separated by ';'
- constructs can be escaped: `\{{var}}` or `asd\;fgh` inside of foreaches will be handled as normal strings

### Limitations and possible next steps
- Only GET is supported currently
- Only tested on linux
- No static resources like images can be sent
- The project is probably not 100% HTTP standard compliant, but it does work with at least firefox 🤪
- The server is single threaded

These problems would be easy to fix if the need for it would present itself.