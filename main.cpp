#include <deque>
#include <future>
#include <map>
#include <string>
#include <thread>
#include <iostream>

std::map<std::string, std::string> db = 
   {
      {"Hola Don Pepito", "Hola Don Jose"},
      {"Vio usted a mi abuela", "A su abuela yo la vi"},
      {"Adios Don Pepito", "Adios Don Jose"},
   };

struct context_t{
   std::string incoming;
   std::promise<std::string> outgoing;
}; 

struct conversation_t
{
   conversation_t(const std::string& _answer)
   :answer(_answer)
   {}
   
   std::string answer;
   std::future<std::string> response;
};

class audience_t
{
public:
   
   std::future<std::string> say(const std::string& message)
   {
      context_t context;
      context.incoming = message;
      messages.push_back(context);
      return std::move(context.outgoing.get_future());
   }

   void listen()
   {
      std::thread worker([this]{
         for (auto& message : messages)
         {
            sleep_and_append(message);
         }
      });
   }
private:
   void sleep_and_append(context_t& ctx)
   {
      sleep(5);
      ctx.outgoing.set_value(db.at(ctx.incoming));         
   }
   std::deque<std::reference_wrapper<context_t>> messages;
};


int main()
{

   audience_t audience;

   std::deque<conversation_t> conversations;

   conversations.emplace_back("Hola Don Pepito");
   conversations.emplace_back("Vio usted a mi abuela");
   conversations.emplace_back("Adios Don Pepito");

   for (auto& conversation: conversations)
   {
      conversation.response = audience.say(conversation.answer);
   }
   
   audience.listen();

   for (auto& conversation: conversations)
   {
      std::cout << conversation.response.get() << std::endl;   
   }
}
