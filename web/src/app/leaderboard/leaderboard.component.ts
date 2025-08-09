import { CommonModule, DecimalPipe } from '@angular/common';
import { Component, effect, inject } from '@angular/core';
import { leaderboard, ScoreService } from '../score.service';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-leaderboard',
  imports: [DecimalPipe, CommonModule],
  templateUrl: './leaderboard.component.html',
  styleUrl: './leaderboard.component.css',
})
export class LeaderboardComponent {
  // TODO: teamScore 顯示紅藍隊目前領先的基地台數
  teamScore: number[] = [6, 4];
  items$!: Observable<leaderboard[]>;
  private scoreService = inject(ScoreService);
  constructor() {
    effect(() => {
      this.items$ = this.scoreService.getLeaderBoard();
    });
  }

  team0Percent(): number {
    return (this.teamScore[0] / (this.teamScore[0] + this.teamScore[1])) * 100;
  }

  team1Percent(): number {
    return (1 - this.teamScore[0] / (this.teamScore[0] + this.teamScore[1])) * 100;
  }
}