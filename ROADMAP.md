# Roadmap for Kalimat

> Original source (Mohammad Samy's blog page): http://web.archive.org/web/20120418221033/http://iamsamy.blogspot.com/2011/08/roadmap-for-kalimat.html

What's the whole plan, present and future, for the Kalimat programming language? Here's a rough outline.

These goals are listed roughly from "Immediate future" to "long term", but they are not strictly in order: Some things are done in parallel, some of the 'long term' items have actually started now, the important part here is the ideas, not the order of their execution.

## More stable language

- Make [FFI](http://web.archive.org/web/20120418221033/http://iamsamy.blogspot.com/2011/08/blog-post_11.html) work correctly, fix memory leaks, pointer issues...etc
- Fix odds & ends in the language, like e.g missing graphics commands
- Make it produce independent applications, either by separating the VM from the IDE or by compiling to .exe (note: work already [started](http://web.archive.org/web/20120418221033/http://iamsamy.blogspot.com/2011/08/how-kalimat-produces-exe-files.html))

## Children & Education

- A site "trykalimat.com" where users can type and run Kalimat programs directly in the web browser, to test the language without downloading.
- A Youtube-like site where children can upload their programs for others to see (the idea comes from MIT Scratch) and a 'share' button in the IDE for automatic uploads
- Integrated tutorials, labs and exercises in the IDE itself
- [Social ecosystem](http://web.archive.org/web/20120418221033/http://iamsamy.blogspot.com/2010/04/blog-post.html) of books, online communities, training...etc
- Programmable toys that have an interface to Kalimat...OOP with real physical objects!
- Research and [computational thinking](http://web.archive.org/web/20120418221033/http://iamsamy.blogspot.com/2010/11/blog-post.html)

## A professional language

- Add more libraries, support for web applications, (perhaps also iPhone or Android applications)
- Optional static type checking
- An online repository for installing libraries and components from the IDE
- Faster VM, better garbage collection, running concurrent processes in parallel
- 'Sister' Compilers for the Java VM or .Net CLR
- A book "Kalimat internals" to explain design & architecture behind Kalimat, to help contributors to the project, beginner compiler writers, and people who want to create their own Arabic PLs
- Transform Kalimat from "Mohamed Samy's project" into "Kalimat team project"

Let's hope at least a significant portion of this is somehow accomplished, إن شاء الله.