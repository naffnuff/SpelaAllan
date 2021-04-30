#include <SFML/Graphics.hpp>

// overridden factories to create our special Sfml and TinyXml based objects
#include "example/override/exampleobjectfactory.h"
#include "example/override/examplefilefactory.h"

// include to use Spriter animations
#include "spriterengine/spriterengine.h"
#include "spriterengine/global/settings.h"

int main()
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);

	// setup Sfml Render Window
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML Spriter Test");


	// setup a clock and a counter for fps
	int fpsCount = 0;
	sf::Clock fpsTimer;


	// setup text objects for fps and object count
	sf::Text fpsText;
	sf::Text objectCountText;
	sf::Font font;

	font.loadFromFile("Content/PT-Sans/PTC55F.ttf");
	fpsText.setFont(font);
	objectCountText.setFont(font);

	fpsText.setColor(sf::Color::Red);
	fpsText.setCharacterSize(20);
	fpsText.setPosition(50, 50);

	objectCountText.setColor(sf::Color::Red);
	objectCountText.setCharacterSize(20);
	objectCountText.setPosition(50, 80);



	// load Spriter file into SpriterModel object using our custom factories
	SpriterEngine::SpriterModel scmlModel("Content/Allan/Allan.scml", new SpriterEngine::ExampleFileFactory(&window), new SpriterEngine::ExampleObjectFactory(&window));
	//SpriterEngine::SpriterModel scmlModel("Release/GreyGuy/player.scon", new SpriterEngine::ExampleFileFactory(&window), new SpriterEngine::ExampleObjectFactory(&window));
	//SpriterEngine::SpriterModel scmlModel("Release/GreyGuyAtlas/GreyGuy.scml", new SpriterEngine::ExampleFileFactory(&window), new SpriterEngine::ExampleObjectFactory(&window));

	// create an array to hold instances of our character
	std::vector<SpriterEngine::EntityInstance*> instances;

	// create a timer to run our animations
	sf::Clock spriterAnimTimer;

	bool leftPressed = false;
	bool rightPressed = false;
	bool firePressed = false;
	bool fireShot = false;
	bool jumpPressed = false;
	bool facingForward = true;

	float currentAnimationTime = 0.0f;
	std::string animationName = "Idle";

	sf::Texture houseTexture;
	houseTexture.loadFromFile("Content/Hus3.png");
	sf::Sprite houseSprite(houseTexture);

	SpriterEngine::EntityInstance* inst = scmlModel.getNewEntityInstance("Allan");
	instances.push_back(inst);

	sf::Vector2f houseLevelPosition(0.0f, window.getSize().y - houseTexture.getSize().y);

	SpriterEngine::point levelPosition(window.getSize().x / 4.0, window.getSize().y - 200.0f);
	SpriterEngine::point screenPosition(window.getSize().x / 4.0, window.getSize().y - 200.0f);

	SpriterEngine::point levelOffset(0.0, 0.0);
	
	while (window.isOpen())
	{

		// if window is closed delete everything and exit
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				for (auto& inst : instances)
				{
					delete inst;
				}

				window.close();
				return 0;
			}
			else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
			{
				bool isPressed = event.type == sf::Event::KeyPressed;
				switch (event.key.code)
				{
				case sf::Keyboard::A:
					leftPressed = isPressed;
					break;
				case sf::Keyboard::D:
					rightPressed = isPressed;
					break;
				case sf::Keyboard::Space:
					jumpPressed = isPressed;
					break;
				case sf::Keyboard::J:
					firePressed = isPressed;
					if (firePressed)
					{
						fireShot = true;
					}
					break;
				default:
					break;
				}
				if (isPressed && (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D))
				{
					facingForward = event.key.code == sf::Keyboard::D;
				}
				currentAnimationTime = 0.0f;
			}
		}

		SpriterEngine::point scale(facingForward ? 0.5 : -0.5f, 0.5f);

		float timeElapsed = spriterAnimTimer.restart().asSeconds() * 1000;

		if (inst)
		{
			currentAnimationTime += timeElapsed;
			if (currentAnimationTime >= 800.0f)
			{
				currentAnimationTime -= 1000.0f;
				if (fireShot && !firePressed)
				{
					fireShot = false;
				}
			}
			std::string nextAnimationName = "Idle";
			if (fireShot)
			{
				nextAnimationName = "Shoot";
			}
			else if (rightPressed || leftPressed)
			{
				nextAnimationName = "Walk";
				levelPosition.x += (rightPressed ? timeElapsed : -timeElapsed) * 0.2;
				screenPosition.x = levelPosition.x + levelOffset.x;
				screenPosition.y = levelPosition.y + levelOffset.y;
				if (screenPosition.x < window.getSize().x / 4.0)
				{
					levelOffset.x += timeElapsed * 0.2;
				}
				if (screenPosition.x > window.getSize().x * 3.0 / 4.0)
				{
					levelOffset.x -= timeElapsed * 0.2;
				}
			}
			if (animationName != nextAnimationName)
			{
				animationName = nextAnimationName;
				inst->setCurrentAnimation(animationName);
				inst->setCurrentTime(0.0f);
			}

			inst->setScale(scale);

			inst->setPosition(screenPosition);

			//inst->setAngle(SpriterEngine::toRadians(rand() % 360));
		}

		sf::Vector2f houseScreenPosition;
		houseScreenPosition.x = houseLevelPosition.x + levelOffset.x;
		houseScreenPosition.y = houseLevelPosition.y + levelOffset.y;
		houseSprite.setPosition(houseScreenPosition);

		// update the object count text with the total number of instances
		objectCountText.setString("object count:  " + std::to_string(instances.size()));

		// update the fps text every second
		if (fpsTimer.getElapsedTime().asSeconds() >= 1)
		{
			fpsText.setString("fps:  " + std::to_string(fpsCount));

			fpsTimer.restart();
			fpsCount = 0;
		}
		++fpsCount;


		// clear sfml window
		window.clear();

		window.draw(houseSprite);

		// update all instances
		for (auto& it : instances)
		{
			if (it)
			{
				// tell the instance how much time has elapsed
				it->setTimeElapsed(timeElapsed);
				it->render();
				// it->playSoundTriggers();
			}
		}

		// render our text
		window.draw(fpsText);
		window.draw(objectCountText);


		// update the sfml window 
		window.display();
	}

	for (auto& it : instances)
	{
		delete it;
	}

	return 0;
}


