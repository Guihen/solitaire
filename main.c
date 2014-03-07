#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CARD.h"
#include "CARD_STACK.h"

#define DECKSIZE 48

Card *get_deck ()
{
  int deck_size = DECKSIZE;
  char *line = malloc (5);
  Card card;
  Card *deck = malloc(sizeof(Card) * DECKSIZE);

  while (deck_size) {
    card = malloc(sizeof(*card));
    fgets(line, 5, stdin); /* <Rank><space><Suit><enter>\0 */
    card->rank = line[0];
    card->suit = line[2];
    card->face_up = false;
    deck[--deck_size] = card;
  }
  return deck;
}

void show_deck (Card *deck)
{
  int i;
  for (i = 0; i < DECKSIZE; i++)
    printf ("%c %c\n", deck[i]->rank, deck[i]->suit);
}

CardStack *init_tableau_stacks()
{
  int i;
  CardStack *tableau_stacks = malloc (sizeof (CardStack) * 7);

  for (i = 0; i < 7; i++)
  {
    tableau_stacks[i] = new_stack();
  }
  return tableau_stacks;
}

CardStack* prepare_tableau_stacks (Card *deck)
{
  int i, j, deck_index = 0;
  CardStack *tableau_stacks = init_tableau_stacks ();

  for (i = 0; i < 7; i++)
    for (j = i; j < 7; j++)
    {
      push (tableau_stacks[j], deck[deck_index]);
      deck[deck_index] = NONE; /* Remove the card from deck */
      deck_index++;
    }

  for (i = 0; i < 7; i++)
    get_card (get_first_node (tableau_stacks[i]))->face_up = true;

  return tableau_stacks;
}

void print_tableau_stacks (CardStack *tableau_stacks)
{
  int i, j;
  Node node[7];
  Card card;
  bool exit_condition;

  for (i = 0; i < 7; i++)
    node[i] = get_last_node (tableau_stacks[i]);

  i = 0, exit_condition = false;
  while (!exit_condition)
  {
    if (i == 0) printf ("| ");

    if (node[i] != NULL)
    {
      card = get_card (node[i]);

      if (card->face_up)
        printf("  [%c%c]  ", card->rank, card->suit);
      else printf("  [??]  ");

      node[i] = previous_node (node[i]);
    }
    else printf("        ");

    if (++i == 7)
    {
      printf ("|\n");
      i = 0;
    }

    /* Exit Condition. */
    for (j = 0; j < 7; j++)
    {
      /* INVARIANT RELATION: all the nodes with index < j are null nodes. */
      if (node[j] != NULL) break;
    }
    if (j == 7) exit_condition = true;
  }
}

CardStack prepare_stock_stack (Card *deck)
{
  int i;
  CardStack stock = new_stack();
  for (i = DECKSIZE - 1; deck[i] != NONE; i--)
  {
    push (stock, deck[i]);
    deck[i] = NONE;
  }
  free(deck);
  return stock;
}

CardStack *prepare_foundation_stacks()
{
  int i;
  CardStack *foundation_stacks = malloc(sizeof(CardStack) * 4);
  for (i = 0; i < 5; i++)
    foundation_stacks[i] = new_stack();
  return foundation_stacks;
}

char *show_stock (CardStack stock)
{
  if (get_first_node(stock) == NULL)
    return "RELOAD";
  return " GET! ";
}

char *show_talon (CardStack talon)
{
  Node top = get_first_node(talon);
  Card card;
  char *msg;

  if ( top == NULL)
    return "XX";

  msg = malloc (3);
  card = get_card (top);
  sprintf(msg, "%c%c", card->rank, card->suit);
  return msg;
}

char *show_foundation_stacks (CardStack *foundation_stacks)
{
  int i;
  char *msg = malloc (4 * 8 + 1),
    *temp = malloc (9);
  char suit;
  Node node;
  Card card;

  for (i = 0; i < 4; i++)
  {
    node = get_first_node (foundation_stacks[i]);
    if (node != NULL)
    {
      card = get_card (node);
      sprintf (temp, "  [%c%c]  ", card->rank, card->suit);
    }
    else
    {
      switch (i)
      {
        case 0: suit = 'C'; break;
        case 1: suit = 'E'; break;
        case 2: suit = 'P'; break;
        case 3: suit = 'O'; break;
      }
      sprintf (temp, "  [  %c]  ", suit);
    } 
    msg = strcat (msg, temp);
  }
  return msg;
}

bool is_red (Suit s)
{
  return (s == 'C' || s == 'O');
}

Rank next_rank (Rank r)
{
  if (r < 57) return r + 1;
  if (r == '9') return 'J';
  if (r == 'J') return 'Q';
  if (r == 'Q') return 'K';
  else return 'X'; /* This will not match with nothing. */
}

bool could_push (Card origin, Card destination)
{
  return ((is_red (origin->suit) != is_red (destination->suit)) &&
      (next_rank (origin->rank) == destination->rank));
}

void print (CardStack stock, CardStack talon, CardStack *foundation_stacks, 
  CardStack *tableau_stacks, char *move_description)
{
  /* This panel contains 59 chars. */
  /* 24 spaces between the title and the form. */
  printf ("+---------------------------------------------------------+\n");
  printf ("|                        Solitaire                        |\n");
  printf ("+---------------------------------------------------------+\n");
  printf ("| (%s) [%s]      %s |\n",
    show_stock(stock), show_talon(talon), 
    show_foundation_stacks(foundation_stacks));
  
  printf ("|                                                         |\n");
  printf ("|                                                         |\n");

  print_tableau_stacks (tableau_stacks);

  printf ("|_________________________________________________________|\n");
  printf (" Next move description: %s\n", move_description);
  printf("\t\t(tap a key to continue...)\n");

  /* it should wait the user hit a key... */
  system("clear");
}

int main (void)
{
  int i, j;
  Card *deck = get_deck();
  char *move_description = malloc (48);
  CardStack *tableau_stacks = prepare_tableau_stacks (deck),
    *foundation_stacks = prepare_foundation_stacks();
  CardStack stock = prepare_stock_stack (deck),
    talon = new_stack();
  bool playing;
  Card card;
  Node node;
  
  playing = true;
  while (playing)
  {
    /* INVARIANT RELATION: tableau stacks with index < i has been analyzed. */
    for (i = 0; i < 7; i++)
    {
      card = get_card (get_first_node (tableau_stacks[i]));

      /* I. R.: tableau stacks with index < j has been visited. */
      for (j = 0; j < 7; j++)
      {
        /* skips search in the same stack. */
        if (j == i) continue;

        if (could_push (card, 
          get_card (get_first_node (tableau_stacks[j]))))
        {
          /* FIX this condition. */
          node = next_node (get_first_node (tableau_stacks[i]));
          if (node != NULL && get_card (node)->face_up)
            continue;

          sprintf (move_description, "%c%c from %d to %d.", 
            card->rank, card->suit, i + 1, j + 1);
          print (stock, talon, foundation_stacks, tableau_stacks, move_description);

          push (tableau_stacks[j], pop (tableau_stacks[i]));
          get_card (get_first_node (tableau_stacks[i]))->face_up = true;
          
          break;
        }
      }

      /* For the invariant relation: j < 7 => a movimentation has happened. */
      if (j < 7) break;
    }

    /* For the invariant relation: i < 7 => a movimentation has happened. */
    if (i < 7) continue;
    
    /*************************** Use the talon ********************************/
    if (empty (talon))
    {
      sprintf (move_description, "Moved a card from Stock to Talon.");
      print (stock, talon, foundation_stacks, tableau_stacks, move_description);

      push (talon, pop (stock));
      continue;
    }

    do
    {
      card = get_card (get_first_node (talon));

      /* I. R.: tableau stacks with index < i has been visited. */
      for (i = 0; i < 7; i++)
      {
        if (could_push (card, get_card (get_first_node (tableau_stacks[i]))))
        {
          sprintf (move_description, "%c%c from Talon to %d.", 
            card->rank, card->suit, i + 1);
          print (stock, talon, foundation_stacks, tableau_stacks, 
            move_description);
          
          card = pop (talon);
          card->face_up = true;
          push (tableau_stacks[i], card);
          break;
        }
      }

      /* For the invariant relation: i = 7 => there is not a move to do. */  
      if (i == 7)
      {
        if (empty (stock))
        {
          print (stock, talon, foundation_stacks, tableau_stacks, "End Game.");
          playing = false;

          break;
        }

        sprintf (move_description, "Moved a card from Stock to Talon.");
        print (stock, talon, foundation_stacks, tableau_stacks, move_description);

        push (talon, pop (stock));
      }

    } while (i == 7);
  }
  
  return 0;
}
