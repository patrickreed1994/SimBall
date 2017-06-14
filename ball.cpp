//CSC432 FALL 2014, Michael Black
//2D BALL MOVEMENT DEMO
//YOU CAN USE THIS AS A BASIS FOR YOUR PROJECT 1
//
//THIS HAS BALLS MOVING AROUND IN 2D AND BOUNCING OFF WALLS.  COLLISIONS ARE NOT YET DETECTED.
//
//A MOUSE LISTENER IS INCLUDED.  ON DOUBLECLICK, A NEW BALL APPEARS.
//
//When you see the words "TODO", you should add code there

//necessary libraries
#include <iostream>
#include <QtGui>
#include <math.h>

//your class definitions
#include "ball.h"
//constants.  these numbers can be adjusted
const int MAXBALLS=1000;	//more than MAXBALLS balls will cause a segmentation fault
const int BOUND=500;		//size of the screen
const int MARGIN=50;		//balls are created at least this far from the edge
const int RADIUS=15;		//all balls start with this radius
const float RESTITUTION=1;	//energy lost in a collision: from 0.0 (inelastic: all is lost) to 1.0 (elastic: nothing is lost)
const float GRAVITY=.3;		//not currently used, but you can add GRAVITY * DT to a ball's y velocity in move() to create a gravity effect
const float DT=0.2;		//smaller = slower simulation = more accurate
const int TIMEUNIT=1000/66;	//timer update function runs 66 times per second

const int MAXLINKS=1000;
Link* links[MAXLINKS];

//globals
int BallCount=0;		//starting with 10 balls.  you can change this.
int LinkCount = 0;		
Ball* balls[MAXBALLS];		//array of ball pointers
QGraphicsScene* thescene;	//the canvas holding the ball graphical objects
BallView* view;			//the window
bool running = false;		//boolean if simulation is running


//ball constructor
//makes a ball at a random position, with a random velocity, and a random mass
Ball::Ball()
{
	setPos(qrand()%(BOUND-MARGIN*2)+MARGIN,qrand()%(BOUND-MARGIN*2)+MARGIN);
	position=new Vector(x(),y());
	mass=(qrand()%10000)/1000.0;

	//the color is random.  use a darker shade to indicate greater mass.
	color.setBlue(0); color.setRed(0); color.setGreen(0);
	switch(qrand()%3)
	{
		case 0:color.setBlue(255-(int)(25*mass)); break;
		case 1:color.setRed(255-(int)(25*mass)); break;
		case 2:color.setGreen(255-(int)(25*mass)); break;
	}

	//choosing an initial velocity with random direction but magnitude 1
	//all balls start out going the same speed
	float speed=qrand()%5+10;
	float angle=(qrand()%360)*2*3.14/360.0;
	velocity=new Vector(speed*cos(angle),speed*sin(angle));
	//all balls have the same radius
	radius=RADIUS;
}

Link::Link(Ball* ball1, Ball* ball2)
{
	firstball=ball1;
	secondball=ball2;
	setPos(ball1->position->x, ball1->position->y);

	float distance = sqrt(ball1->position->sub(ball2->position)->lengthsquared());
	length = distance;
	color.setBlue(255); color.setRed(255); color.setGreen(255);
}

//where to update screen relative to a ball's center position
//you shouldn't need to change this
QRectF Ball::boundingRect() const
{
	return QRectF(-radius,-radius,radius*2,radius*2);
}

//called by qt system to paint the ball when it moves.  you shouldn't need to change this
void Ball::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setBrush(color);
	painter->drawEllipse(-radius,-radius,radius*2,radius*2);
}

// EDIT THIS PART TOMORROW - 10/1
QRectF Link::boundingRect() const
{
	int diagonal = (RADIUS + RADIUS) + length;
	return QRectF(-diagonal-5,-diagonal-5,2*diagonal+5,2*diagonal+5);///////needs to be a relative position therefore constant
	
}


//called by qt system to paint the ball when it moves.  you shouldn't need to change this
void Link::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	int firstballx = (int) (firstball->position->x);
	int firstbally = (int) (firstball->position->y);
	int secondballx = (int) (secondball->position->x);
	int secondbally = (int) (secondball->position->y);
	
	int Xdistance=secondballx-firstballx;
	int Ydistance=secondbally-firstbally;
	
	painter->setBrush(color);
	painter->drawLine(Xdistance,Ydistance,0,0);
}

//called when a collision happens.  your code will go in here
void handleCollision(Ball* ball1, Ball* ball2)
{
	std::cout << "Collision happened" << std::endl;
	Vector* Xcollision = ball1->position->sub(ball2->position);
	float distance = sqrt(Xcollision->lengthsquared());
	float adjust = (ball1->radius + ball2->radius - distance)/distance;
	Vector* Xadjust = ball1->position->sub(ball2->position)->mul(adjust);
	
	Vector* Xadjust1 = Xadjust->mul(((1/ball1->mass)/(1/ball1->mass + 1/ball2->mass)));
	Vector* Xadjust2 = Xadjust->mul(((1/ball2->mass)/(1/ball1->mass + 1/ball2->mass)));	

	ball1->position = ball1->position->add(Xadjust1);
	ball2->position = ball2->position->sub(Xadjust2);
	Xcollision=Xcollision->normalize();

	Vector* Vdiff = ball1->velocity->sub(ball2->velocity);
	
	float vadjust = Vdiff->dot(Xcollision);
	float vadjust1 = (1 + RESTITUTION) * vadjust * ((1/ball1->mass)/(1/ball1->mass + 1/ball2->mass));
	float vadjust2 = (1 + RESTITUTION) * vadjust * ((1/ball2->mass)/(1/ball1->mass + 1/ball2->mass));

	ball1->velocity = ball1->velocity->sub(Xcollision->mul(vadjust1));
	ball2->velocity = ball2->velocity->add(Xcollision->mul(vadjust2));

	delete Xcollision;
	delete Xadjust;
	delete Xadjust1;
	delete Xadjust2;
	delete Vdiff;


}

void handleLink(Link* link)
{
	Ball* ball1=link->firstball;
	Ball* ball2=link->secondball;

	Vector* Xcollision = ball1->position->sub(ball2->position);
	float distance = sqrt(Xcollision->lengthsquared());
	float adjust = (-distance + link->length)/distance;
	Vector* Xadjust = ball1->position->sub(ball2->position)->mul(adjust);
	
	Vector* Xadjust1 = Xadjust->mul(((1/ball1->mass)/(1/ball1->mass + 1/ball2->mass)));
	Vector* Xadjust2 = Xadjust->mul(((1/ball2->mass)/(1/ball1->mass + 1/ball2->mass)));	

	ball1->position = ball1->position->add(Xadjust1);
	ball2->position = ball2->position->sub(Xadjust2);
	Xcollision=Xcollision->normalize();

	Vector* Vdiff = ball1->velocity->sub(ball2->velocity);
	
	float vadjust = Vdiff->dot(Xcollision);
	float vadjust1 = (1 + 1) * vadjust * ((1/ball1->mass)/(1/ball1->mass + 1/ball2->mass));
	float vadjust2 = (1 + 1) * vadjust * ((1/ball2->mass)/(1/ball1->mass + 1/ball2->mass));

	ball1->velocity = ball1->velocity->sub(Xcollision->mul(vadjust1));
	ball2->velocity = ball2->velocity->add(Xcollision->mul(vadjust2));

	delete Xcollision;
	delete Xadjust;
	delete Xadjust1;
	delete Xadjust2;
	delete Vdiff;

}

//move a ball one timestep foreward
void Ball::move()
{
	position->x+=velocity->x*DT;
	position->y+=velocity->y*DT;
	velocity->y += DT*GRAVITY;

}

//sets up timer.  don't change this.
TimerHandler::TimerHandler(int t)
{
	QTimer* timer=new QTimer(this);
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer->start(t);
}

//called automatically 66 times per second (you can adjust this by changing TIMESTEP at the top of the program)
void TimerHandler::onTimer()
{
	int i,j;

	if(running == true)
	{
		//move each ball
		for(i=0; i<BallCount; i++)
			balls[i]->move();

		//detect collision between balls
		for(i=0; i<BallCount; i++)
		{
			for(j=i+1; j<BallCount; j++)
			{
			
				float distance1 = sqrt(balls[i]->position->sub(balls[j]->position)->lengthsquared());
				if(distance1 <= balls[i]->radius + balls[j]->radius)
					handleCollision(balls[i], balls[j]);
			}
		}

		//detect collision with the boundary walls
		//TODO:  you may want to add restitution to these collisions as well
		for(i=0; i<BallCount; i++)
		{
			//Bottom
			if(balls[i]->position->y + balls[i]->radius >= BOUND)
			{
				balls[i]->position->y = BOUND-balls[i]->radius;
				if(balls[i]->velocity->y > 0)
					balls[i]->velocity->y = -balls[i]->velocity->y;
			}
			//Right
			if(balls[i]->position->x + balls[i]->radius >= BOUND)
			{
				balls[i]->position->x = BOUND-balls[i]->radius;
				if(balls[i]->velocity->x > 0)
					balls[i]->velocity->x = -balls[i]->velocity->x;
			}
			//Top
			if(balls[i]->position->y - balls[i]->radius <=0)
			{
				balls[i]->position->y = balls[i]->radius;
				if(balls[i]->velocity->y < 0)
					balls[i]->velocity->y = -balls[i]->velocity->y;
			}
			//Left
			if(balls[i]->position->x - balls[i]->radius <=0)
			{
				balls[i]->position->x = balls[i]->radius;
				if(balls[i]->velocity->x < 0)
					balls[i]->velocity->x = -balls[i]->velocity->x;
			}
		}
		
		for(int c=0; c < LinkCount; c++)
			handleLink(links[c]);

		//draws and sets balls
		for(i=0; i<BallCount; i++)
			balls[i]->setPos(balls[i]->position->x,balls[i]->position->y);
	
		//draws links
		for(int b=0; b<LinkCount; b++)
		{
			int firstballx = (int) (links[b]->firstball->position->x);
			int firstbally = (int) (links[b]->firstball->position->y);
			links[b]->setPos(firstballx,firstbally);			
		}
	}

	
}

//constructor for the window.  you can leave it empty if you want.
BallView::BallView(QGraphicsScene *scene, QWidget* parent):QGraphicsView(scene, parent)
{
}

//this is called when the mouse is pressed.  use it if you want.
void BallView::mousePressEvent(QMouseEvent *event)
{
	QPointF startPt = event->pos();
	pressX = startPt.x();
	pressY = startPt.y();	
}

//this is called when the mouse is released.  use it if you want.
void BallView::mouseReleaseEvent(QMouseEvent *event)
{
	QPointF endPt = event->pos();
	releaseX = endPt.x();
	releaseY = endPt.y();

	for(int c = 0; c < BallCount;c++)
	{
		if((pressY <= balls[c]->position->y + (RADIUS * 2)) and (pressY >= balls[c]->position->y))
		{
			if((pressX <= balls[c]->position->x + (RADIUS * 2)) and (pressX >= balls[c]->position->x))		
			{
				for(int d = 0; d < BallCount;d++)
				{
					if((releaseY <= balls[d]->position->y + (RADIUS * 2)) and (releaseY >= balls[d]->position->y) and balls[c] != balls[d])
					{
						if((releaseX <= balls[d]->position->x + (RADIUS * 2)) and (releaseX >= balls[d]->position->x))		
						{
							std::cout<<"connect"<<std::endl;
							links[LinkCount]=new Link(balls[c], balls[d]);	
							thescene->addItem(links[LinkCount]);
							LinkCount+=1;
							
						}
					}
			
				}
			}
			
		}
	}

}

//this is called when the mouse is doubleclicked.
//currently I have it just creating a new ball at that point
//you can remove that and change this to whatever you like
void BallView::mouseDoubleClickEvent(QMouseEvent *event)
{
	//TODO: modify this if you want to...

	if(event->button() == Qt::LeftButton)
	{
		balls[BallCount]=new Ball;
		balls[BallCount]->position->x=event->x()-RADIUS;
		balls[BallCount]->position->y=event->y()-RADIUS;
		balls[BallCount]->velocity->x=balls[BallCount]->velocity->y=0;
		thescene->addItem(balls[BallCount]);
		balls[BallCount]->setPos(balls[BallCount]->position->x,balls[BallCount]->position->y);
		BallCount++;
	}

	//double right click to stop and start time	
	if(event->button() == Qt::RightButton and running == true)
	{
		std::cout<<"pause"<<std::endl;
		running = false;
	}
	else if(event->button() == Qt::RightButton and running == false)
	{
		std::cout<<"run"<<std::endl;
		running = true;
	}
}

//the program starts here
int main(int argc, char **argv)
{
	//this will keep the program running even after main ends
	QApplication app(argc,argv);
	//seed random number generator to current time
	qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

	//make a scene
	thescene=new QGraphicsScene();
	thescene->setSceneRect(0,0,BOUND,BOUND);

	//make the balls and add them to the scene
	for(int i=0; i<BallCount; i++)
	{
		balls[i] = new Ball;
		thescene->addItem(balls[i]);
	}
	


	//TODO:
	//if you want different balls, with different radii or behavior, such as pistons,
	//create them here

	//make the window
	view=new BallView(thescene);
	view->setWindowTitle("Ball");
	view->resize(BOUND+50,BOUND+50);
	view->show();
	view->setMouseTracking(true);

	//start the timer going.  onTimer will now start running periodically
	TimerHandler timer(TIMEUNIT);

	//make everything appear and end main
	return app.exec();
}
