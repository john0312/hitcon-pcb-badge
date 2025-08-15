import { CommonModule, DecimalPipe } from '@angular/common';
import { Component, OnDestroy, OnInit } from '@angular/core';
import { leaderboard, ScoreService } from '../score.service';
import { LittleEndianPipe } from '../little-endian.pipe';
import { StationsService as StationService } from '../stations.service';
import { env } from '../../config';

@Component({
  selector: 'app-leaderboard',
  imports: [DecimalPipe, CommonModule, LittleEndianPipe],
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
    this.teamScore = [0, 0, 0];
    this.scoreService.getLeaderBoard().subscribe((scores) => {
      this.items = scores;
    });
    this.stationService.getStationScore().subscribe((scores) => {
      scores.forEach((score) => {
        if (score > this.threshold) {
          this.teamScore[0]++;
        } else if (score < -this.threshold) {
          this.teamScore[1]++;
        } else {
          this.teamScore[2]++;
        }
      });
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