import { AfterViewInit, Component, ElementRef, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { CommonModule } from '@angular/common';
import { StateService } from '../state.service';
import { AbsPipe } from '../abs.pipe';
import { StationsService } from '../stations.service';
import { env } from '../../config';

@Component({
  selector: 'app-home',
  imports: [CommonModule, AbsPipe],
  templateUrl: './home.component.html',
  styleUrl: './home.component.css'
})
export class HomeComponent implements AfterViewInit, OnInit, OnDestroy {
  @ViewChild('mapWrapper') mapWrapper?: ElementRef;

  readonly scalerMax = 299;
  readonly scalerMin = 79;
  readonly threshold = env.score.threshold;
  floor = 0;
  scaler = 99;
  readonly floorImg: string[] = ['./3f.svg', './4f.svg', './hitcon-hat-logo.svg'];

  constructor(private state: StateService, private stationService: StationsService) {
    this.floor = this.state.floor;
    this.scaler = this.state.scaler;
    this.updateScore();
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private intervalID?: any;

  ngOnInit(): void {
    this.intervalID = setInterval(() => {
      this.updateScore();
    }, env.api.period);
  }

  ngOnDestroy(): void {
    if (this.intervalID) {
      clearInterval(this.intervalID);
    }
  }

  points0: point[] = [
    {
      x: 48,  // 第二會議室左邊外面
      y: 12.5,
      score: 0,
      id: 14,
    },
    {
      x: 43,  // 第二會議室最左
      y: 20.5,
      score: 0,
      id: 11,
    },
    {
      x: 35,  // 第二會議外右上
      y: 36.5,
      score: 0,
      id: 9,
    },
    {
      x: 77, // 國際會議廳左下
      y: 32.5,
      score: 0,
      id: 10,
    },
    {
      x: 15, // 遠距會議室左側外面
      y: 60,
      score: 0,
      id: 12,
    },
    {
      x: 15.5, // 遠距會議室靠門
      y: 68,
      score: 0,
      id: 13,
    },
    {
      x: 40,  // 國際會議廳右
      y: 63,
      score: 0,
      id: 4,
    },
    {
      x: 38.5, // 第一會議室右
      y: 79.5,
      score: 0,
      id: 7,
    },
    {
      x: 48, // 第一會議室右外
      y: 87.2,
      score: 0,
      id: 8,
    },
    {
      x: 80, // 第一會議室左下外側
      y: 71,
      score: 0,
      id: 6,
    },
  ];

  points1: point[] = [
    {
      x: 91,  // 大會攤位左邊
      y: 38,
      score: 0,
      id: 15,
    },
    {
      x: 91,  // 大會攤位右邊
      y: 61,
      score: 0,
      id: 16,
    },
    {
      x: 80,  // 國際會議廳左下
      y: 50,
      score: 0,
      id: 2,
    },
    {
      x: 79,  // 國際會議廳右下
      y: 58,
      score: 0,
      id: 3,
    },
    
    {
      x: 65,  // 南棟下面
      y: 33.5,
      score: 0,
      id: 20,
    },
    {
      x: 41,  // 南棟上面
      y: 37.5,
      score: 0,
      id: 19,
    },
    {
      x: 65,  // 北棟下面
      y: 65.8,
      score: 0,
      id: 18,
    },
    {
      x: 41,  // 北棟上面
      y: 62,
      score: 0,
      id: 17,
    }
  ];

  // 雲端 base-station
  points2: point[] = [
    {
      x: 50,  // 中間點
      y: 50,
      score: 0,
      id: 21,
    }
  ];

  points = [
    this.points0,
    this.points1,
    this.points2,
  ];

  ngAfterViewInit(): void {
    this.moveWrapper(this.state.scrollLeft, this.state.scrollTop);
  }

  private updateScore() {
    this.stationService.getStationScore().subscribe((fetched: number[]) => {
      this.points0.forEach((point, index) => {
        this.points0[index].score = fetched[point.id];
      });
      this.points1.forEach((point, index) => {
        this.points1[index].score = fetched[point.id];
      });
      this.points2.forEach((point, index) => {
        this.points2[index].score = fetched[point.id];
      });
      this.points[0] = this.points0;
      this.points[1] = this.points1;
      this.points[2] = this.points2;
    });
  }

  changeFloor(floor: number) {
    this.floor = floor;
    this.state.floor = this.floor;
  }

  zoom(delta: number) {
    this.scaler = Math.max(Math.min(this.scaler + delta, this.scalerMax), this.scalerMin);
    this.state.scaler = this.scaler;
  }

  moveWrapper(left: number | null, top: number | null) {
    if (!this.mapWrapper) {
      return;
    }
    const el = this.mapWrapper.nativeElement;
    if (left != null && top != null) {
      el.scrollLeft = Math.max(0, left);
      el.scrollTop = Math.max(0, top);
    } else {
      el.scrollLeft = Math.max(0, (el.scrollWidth - el.offsetWidth) / 2);
      el.scrollTop = Math.max(0, (el.scrollHeight - el.offsetHeight) / 2);
    }
  }

  onMapWrapperScroll(event: Event) {
    const target = event.target as HTMLElement;
    if (this.mapWrapper && this.mapWrapper.nativeElement === target) {
      this.state.scrollLeft = target.scrollLeft;
      this.state.scrollTop = target.scrollTop;
    }
  }

  // convert score to width on UI
  // lowest score: -30000
  // highest score: 30000
  // when the score is positive red > blue
  // when the score is negative blue > red
  // scoreToRingRadius represents the blue team's score
  scoreToRingRadius(score: number): number {
    return (env.score.maximum - score) / (env.score.maximum * 2) * 360;
  }
}

interface point {
  score: number // for each team score
  x: number // x-axis of position
  y: number // y-axis of position
  id: number
}
