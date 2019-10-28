#include "TestLib.h"

using namespace sqfish;

// funzione con un parametro intero che restituisce un intero
int pippo(int x) {
	return x * x;
}

// due funzioni overloaded (stesso nome, parametri diversi)
std::string pluto(const char *a) {
	return std::string("Hai chiamato 'pluto(\"") + a + "\")'";
}

std::string pluto(const char *a, const char *b) {
	return std::string("Hai chiamato 'pluto(\"") + a + "\", \"" + b + "\")'";
}

// altre due funzioni overloaded con parametri diversi
int  fact(int i) {
	if(i <= 1)
		return 1;
	return i * fact(i - 1);
}

float  fact(float i) {
	if(i <= 1)
		return 1;
	return i * fact(i - 1);
}

// una classe di prova
class TestClass {
	private:
		
	protected:
		
	public:
		
		// costruttori
		TestClass() { aVar = 0; aString = "empty"; }
		TestClass(int v) { aVar = v; aString = "cucu"; }
		
		// distruttore
		virtual  ~TestClass() {}
		
		// un paio di variabili membro
		int aVar;
		std::string aString;
		
		// una variabile statica
		static int staticVar;
		
		// una funzione virtuale
		virtual int  aFunc(int a, int b) {
			return aVar * a * b;
		}
		
		// una funzione statica
		static int  staticFunc() {
			return 5;
		}
};

// una classe derivata
class TestDerived : public TestClass {
	public:
		
		// costruttori
		TestDerived(int i) : TestClass(i) {}
		TestDerived() : TestClass() {}

		// una funzione virtuale che rimpiazza quella
		// della classe parent
		virtual int  aFunc(int a, int b) { return 2 * aVar + a + b; }
};

// la definizione della variabile statica
int TestClass::staticVar = 77;

// due oggetti delle due classi
TestClass test1;
TestDerived test2(77);

// il collegamento a Squirrel
SQUIRREL_API SQInteger registerTestLib(HSQUIRRELVM v) {

	// il tutto va nella tabella radice (RootTable)
	RootTable(v)
	
		// aggiungo la funzione pippo
		// non ho bisogno di specificare i parametri
		.Func("pippo"_LIT, pippo)
		
		// aggiungo le 2 funzioni pluto
		// qui DEVO specificare i parametri per poterle differenziare!
		// il primo "parametro" è il tipo di ritorno della funzione
		.Func<std::string, const char *>("pluto"_LIT, pluto)
		.Func<std::string, const char *, const char *>("pluto"_LIT, pluto)

		// aggiungo le 2 funzioni 'fact'
		// (calcolano il fattoriale di un numero)
		.Func<int, int>("fact"_LIT, fact)
		.Func<float, float>("fact"_LIT, fact)
		
		// aggiungo la prima classe
		.Class<TestClass>("TestClass"_LIT)
		
			// aggiungo i 2 costruttori della classe
			// il primo senza parametri, il secondo
			// con un parametro intero
			.Constructor()
			.Constructor<int>()
			
			// aggiungo le 3 variabili
			.Var("aVar"_LIT, &TestClass::aVar)
			.Var("aString"_LIT, &TestClass::aString)
			.Var("staticVar"_LIT, &TestClass::staticVar)
			
			// aggiungo le funzioni
			.Func("aFunc"_LIT, &TestClass::aFunc)
			.Func("staticFunc"_LIT, &TestClass::staticFunc)
			
			// torno indietro di un livello
			--
		// aggiungo la classe derivata
		// attenzione, tra parentesi occorre inserire la classe parent!
		.Class<TestDerived>("TestClass"_LIT, "TestDerived"_LIT)
		
			// aggiungo i costruttori
			.Constructor<int>()
			.Constructor()
		
			// qui non serve aggiungere la funzione aFunc
			// c'è già dalla classe parent
			
			// torno indietro
			--
			
		// aggiungo le due istanze delle due classi, già create in C++
		.Instance("test1"_LIT, "TestClass"_LIT, test1)
		.Instance("test2"_LIT, "TestDerived"_LIT, test2)
		
		// aggiungo un paio di valori nella tabella root
		.Value("nome"_LIT, "massimo")
		.Value("cognome"_LIT, "del fedele");
		
	// fine (attenzione al ; finale!)
	;
		
	return 1;
}
