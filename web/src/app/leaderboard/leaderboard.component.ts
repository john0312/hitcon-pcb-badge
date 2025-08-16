import { CommonModule, DecimalPipe } from '@angular/common';
import { Component, OnDestroy, OnInit } from '@angular/core';
import { leaderboard, ScoreService } from '../score.service';
import { StationsService as StationService } from '../stations.service';
import { env } from '../../config';

@Component({
  selector: 'app-leaderboard',
  imports: [DecimalPipe, CommonModule],
  templateUrl: './leaderboard.component.html',
  styleUrl: './leaderboard.component.css',
})
export class LeaderboardComponent implements OnInit, OnDestroy {
  readonly threshold = env.score.threshold;
  readonly sponsorIDs = env.score.sponsorIDs;
  readonly sponsorNames = env.score.sponsorNames;
  teamScore: number[] = [0, 0, 0];
  items: leaderboard[] = [];
  constructor(private scoreService: ScoreService, private stationService: StationService) {
    this.update();
  }

  private update() {
    this.scoreService.getLeaderBoard().subscribe((scores) => {
      // in case of fetching error, do not update the data
      if (scores.length != 0) {
        this.items = scores;
      }
    });
    this.stationService.getStationScore().subscribe((scores) => {
      if (scores.length != 0) {
        const teamScore = [0, 0, 0];
        scores.forEach((score) => {
          if (score > this.threshold) {
            teamScore[0]++;
          } else if (score < -this.threshold) {
            teamScore[1]++;
          } else {
            teamScore[2]++;
          }
        });
        this.teamScore = teamScore;
      }
    });
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private intervalID: any;
  ngOnInit(): void {
    this.intervalID = setInterval(() => {
      this.update();
    }, env.api.period);
  }

  ngOnDestroy(): void {
    clearInterval(this.intervalID);
  }

  team0Percent(): number {
    return (this.teamScore[0] / (this.teamScore[0] + this.teamScore[1] + this.teamScore[2])) * 100;
  }

  team1Percent(): number {
    return (this.teamScore[1] / (this.teamScore[0] + this.teamScore[1] + this.teamScore[2])) * 100;
  }

  // when the score is between [-threshold, threshold]
  teamTiePercent(): number {
    return (1 - (this.teamScore[0] + this.teamScore[1]) / (this.teamScore[0] + this.teamScore[1] + this.teamScore[2])) * 100;
  }
}