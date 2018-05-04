all: ad5933

ad5933: 
	clang++ -g -O0 main.cpp -std=c++1z -lcyusb -o ad5933

clean:
	rm ad5933
