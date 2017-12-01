default: lab3b

lab3b: src/lab3b/Lab3b.java src/lab3b/infoobject/*.java
	javac -d ./src/ $^

.PHONY: default clean dist

clean:
	rm -f lab3a src/lab3b/Lab3b.class src/lab3b/infoobject/*.class *.tar.gz

dist:
	tar -czf lab3a-404751542.tar.gz $^
